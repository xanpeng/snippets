/*
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

#include "tree.h"
#include "worker.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORKERS 8192

#define WORKER_SUCCESS ((void*)(intptr_t)0)
#define WORKER_ERROR ((void*)(intptr_t)1)

/** Represents the state of a worker thread
*/
enum worker_state_t
{
  WORKER_STATE_UNINITIALIZED = 0,
  WORKER_STATE_RUNNING = 1,
  WORKER_STATE_STOPPED = 2,
  WORKER_STATE_STOPPED_ERROR = 3,
};

/** Represents a worker thread.
*/
struct worker
{
  /** The next free worker in the worker free list */
  struct worker *next_free_worker;

  /** The name of this worker. It doesn't have to be unique; it's mainly
   * for debugging */
  char name[WORKER_NAME_MAX];

  /** The lock that protects this worker's queue and state */
  pthread_mutex_t lock;

  /** The worker thread */
  pthread_t thread;

  /** The worker state */
  enum worker_state_t state;

  /** Pointer to the head of the message queue */
  struct worker_msg *msg_head;

  /** Pointer to the tail of the message queue */
  struct worker_msg *msg_tail;

  /** Condition variable used to wait for a message */
  pthread_cond_t cond;
};

struct worker_thread_param
{
  struct worker *worker;
  worker_fn_t fn;
  worker_shutdown_fn_t sfn;
  void *data;
};

/** Actual storage for workers */
static struct worker g_workers[MAX_WORKERS];

/** The head of the free workers list, or NULL. */
static struct worker* g_next_free_worker;

/** Protects the free workers list */
static pthread_mutex_t g_next_free_worker_lock;

void* worker_main(void *v)
{
  int done = 0;
  struct worker_msg *msg;
  void *ret = WORKER_SUCCESS;
  struct worker_thread_param *wtp = (struct worker_thread_param *)v;
  struct worker *worker = wtp->worker;
  worker_fn_t fn = wtp->fn;
  worker_shutdown_fn_t sfn = wtp->sfn;
  void *data = wtp->data;
  free(wtp);

  do {
    int res;
    pthread_mutex_lock(&worker->lock);
    while (!worker->msg_head) {
      pthread_cond_wait(&worker->cond, &worker->lock);
    }
    msg = worker->msg_head;
    if (msg->next) {
      worker->msg_head = msg->next;
    }
    else {
      worker->msg_head = NULL;
      worker->msg_tail = NULL;
    }
    msg->next = NULL;
    pthread_mutex_unlock(&worker->lock);
    switch (msg->ty) {
    case WORKER_MSG_SHUTDOWN:
      ret = WORKER_SUCCESS;
      done = 1;
      break;
    default:
      res = fn(msg, data);
      if (res != 0) {
        ret = WORKER_ERROR;
        done = 1;
        break;
      }
    }
    free(msg);
  } while (!done);
  pthread_mutex_lock(&worker->lock);
  if (ret == WORKER_SUCCESS)
    worker->state = WORKER_STATE_STOPPED;
  else
    worker->state = WORKER_STATE_STOPPED_ERROR;
  for (msg = worker->msg_head; msg; ) {
    struct worker_msg *next = msg->next;
    free(msg);
    msg = next;
  }
  worker->msg_head = worker->msg_tail = NULL;
  pthread_mutex_unlock(&worker->lock);
  /* Invoke shutdown function, if it's present. */
  if (sfn) {
    sfn(data);
  }
  return ret;
}

static int worker_msg_compare(const struct worker_msg *ma,
    const struct worker_msg *mb)
{
  if (ma->defer_until.tv_sec < mb->defer_until.tv_sec)
    return -1;
  else if (ma->defer_until.tv_sec > mb->defer_until.tv_sec)
    return 1;
  else if (ma->defer_until.tv_nsec < mb->defer_until.tv_nsec)
    return -1;
  else if (ma->defer_until.tv_nsec > mb->defer_until.tv_nsec)
    return 1;
  else if ((uintptr_t)ma < (uintptr_t)mb)
    return -1;
  else if ((uintptr_t)ma > (uintptr_t)mb)
    return 1;
  return 0;
}

RB_HEAD(defer_msg, worker_msg);
RB_GENERATE(defer_msg, worker_msg, defer_entry, worker_msg_compare);
/** deferred messages, sorted by wakeup time */
static struct defer_msg g_deferred;
/** lock protecting g_deferred */
static pthread_mutex_t g_deferred_lock;
/** condition variable do_deferred_work uses to wait for changes in
 * g_deferred */
static pthread_cond_t g_deferred_cond;
/** Thread that handles sending deferred messages */
static pthread_t g_deferred_msg_thread;

static void *run_deferred_msg_thread(void *v __attribute__((unused)))
{
  struct worker_msg *m;
  struct timespec cur;

  pthread_mutex_lock(&g_deferred_lock);
  while (1) {
    m = RB_MIN(defer_msg, &g_deferred);
    if (!m) {
      pthread_cond_wait(&g_deferred_cond, &g_deferred_lock);
      continue;
    }
    clock_gettime(CLOCK_REALTIME, &cur);
    if ((m->defer_until.tv_sec < cur.tv_sec) &&
        (m->defer_until.tv_nsec < cur.tv_nsec)) {
      RB_REMOVE(defer_msg, &g_deferred, m);
      pthread_mutex_unlock(&g_deferred_lock);
      worker_sendmsg_or_free(m->dst, m);
      pthread_mutex_lock(&g_deferred_lock);
      continue;
    }
    pthread_cond_timedwait(&g_deferred_cond, &g_deferred_lock,
        &m->defer_until);
  }
  return NULL;
}

void worker_sendmsg_deferred(struct worker *w, void *m,
    const struct timespec *ts)
{
  struct worker_msg *msg = (struct worker_msg*)m;

  msg->dst = w;
  msg->defer_until.tv_sec = ts->tv_sec;
  msg->defer_until.tv_nsec = ts->tv_nsec;
  pthread_mutex_lock(&g_deferred_lock);
  RB_INSERT(defer_msg, &g_deferred, msg);
  if (RB_MIN(defer_msg, &g_deferred) == msg)
    pthread_cond_signal(&g_deferred_cond);
  pthread_mutex_unlock(&g_deferred_lock);
}

void worker_sendmsg_deferred_ms(struct worker *w, void *m, int ms)
{
  struct timespec ts;
  int s;

  clock_gettime(CLOCK_REALTIME, &ts);
  s = ms / 1000;
  ts.tv_sec += s;
  ms -= (s * 1000);
  ts.tv_nsec += (ms * 1000);
  if (ts.tv_nsec > 1000000000L) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000L;
  }
  worker_sendmsg_deferred(w, m, &ts);
}

void clear_deferred(void)
{
  struct worker_msg *m, *m_tmp;

  pthread_mutex_lock(&g_deferred_lock);
  RB_FOREACH_SAFE(m, defer_msg, &g_deferred, m_tmp) {
    RB_REMOVE(defer_msg, &g_deferred, m);
    free(m);
  }
  pthread_mutex_unlock(&g_deferred_lock);
}

void worker_init(void)
{
  int ret, i;
  memset(g_workers, 0, sizeof(struct worker) * MAX_WORKERS);
  RB_INIT(&g_deferred);
  ret = pthread_mutex_init(&g_deferred_lock, NULL);
  if (ret)
    abort();
  ret = pthread_cond_init(&g_deferred_cond, NULL);
  if (ret)
    abort();
  ret = pthread_create(&g_deferred_msg_thread, NULL,
      run_deferred_msg_thread, NULL);
  if (ret)
    abort();
  pthread_mutex_init(&g_next_free_worker_lock, NULL);
  for (i = 0; i < MAX_WORKERS; ++i) {
    ret = pthread_mutex_init(&g_workers[i].lock, NULL);
    if (ret)
      abort();
    ret = pthread_cond_init(&g_workers[i].cond, NULL);
    if (ret)
      abort();
  }
  for (i = 0; i < MAX_WORKERS; ++i) {
    g_workers[i].next_free_worker = &g_workers[i + 1];
  }
  g_next_free_worker = &g_workers[0];
  g_workers[MAX_WORKERS - 1].next_free_worker = NULL;
}

struct worker *worker_start(const char *name, worker_fn_t fn,
    worker_shutdown_fn_t sfn, void *data)
{
  int ret;
  struct worker_thread_param *wtp;
  struct worker *worker;
  enum worker_state_t old_state;

  wtp = static_cast<worker_thread_param*>(calloc(1, sizeof(struct worker_thread_param)));
  if (!wtp) {
    return NULL;
  }
  pthread_mutex_lock(&g_next_free_worker_lock);
  if (g_next_free_worker == NULL) {
    pthread_mutex_unlock(&g_next_free_worker_lock);
    free(wtp);
    return NULL;
  }
  worker = g_next_free_worker;
  g_next_free_worker = worker->next_free_worker;
  worker->next_free_worker = NULL;
  snprintf(worker->name, WORKER_NAME_MAX, "%s", name);
  old_state = worker->state;
  worker->state = WORKER_STATE_RUNNING;
  worker->msg_head = NULL;
  worker->msg_tail = NULL;
  wtp->worker = worker;
  wtp->fn = fn;
  wtp->sfn = sfn;
  wtp->data = data;
  ret = pthread_create(&worker->thread, NULL, worker_main, wtp);
  if (ret) {
    worker->name[0] = '\0';
    worker->state = old_state;
    worker->next_free_worker = g_next_free_worker;
    g_next_free_worker = worker;
    pthread_mutex_unlock(&g_next_free_worker_lock);
    return NULL;
  }
  pthread_mutex_unlock(&g_next_free_worker_lock);
  return worker;
}

int worker_sendmsg(struct worker *worker, void *m)
{
  struct worker_msg *msg = (struct worker_msg *)m;
  pthread_mutex_lock(&worker->lock);
  if (worker->state != WORKER_STATE_RUNNING) {
    pthread_mutex_unlock(&worker->lock);
    return -EINVAL;
  }
  if (worker->msg_tail) {
    worker->msg_tail->next = msg;
    worker->msg_tail = msg;
  }
  else {
    worker->msg_head = msg;
    worker->msg_tail = msg;
  }
  pthread_cond_signal(&worker->cond);
  pthread_mutex_unlock(&worker->lock);
  return 0;
}

int worker_sendmsg_or_free(struct worker *worker, void *m)
{
  int ret = worker_sendmsg(worker, m);
  if (ret) {
    free(m);
  }
  return ret;
}

int worker_stop(struct worker *worker)
{
  struct worker_msg *msg = static_cast<worker_msg*>(calloc(1, sizeof(struct worker_msg)));
  if (!msg) {
    return -ENOMEM;
  }
  msg->ty = WORKER_MSG_SHUTDOWN;
  return worker_sendmsg(worker, msg);
}

int worker_join(struct worker *worker)
{
  int ret;
  void *retval;
  pthread_t thread;
  pthread_mutex_lock(&worker->lock);
  if (worker->state == WORKER_STATE_UNINITIALIZED) {
    pthread_mutex_unlock(&worker->lock);
    return -EINVAL;
  }
  thread = worker->thread;
  pthread_mutex_unlock(&worker->lock);
  ret = pthread_join(thread, &retval);
  if (ret) {
    return ret;
  }
  worker->name[0] = '\0';

  pthread_mutex_lock(&g_next_free_worker_lock);
  worker->next_free_worker = g_next_free_worker;
  g_next_free_worker = worker;
  pthread_mutex_unlock(&g_next_free_worker_lock);

  if (retval == WORKER_SUCCESS) {
    return 0;
  }
  else {
    return 1;
  }
}
