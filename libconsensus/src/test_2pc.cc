/*
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

// #define _GNU_SOURCE /* for TEMP_FAILURE_RETRY */

#include "limits.h"
#include "util.h"
#include "worker.h"

#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/************************ constants ********************************/
enum {
  MMM_DO_PROPOSE = 2000,
  MMM_PROPOSE = 3000,
  MMM_RESP,
  MMM_COMMIT_OR_ABORT,
};

enum node_state {
  NODE_STATE_INIT = 0,
  NODE_STATE_COORD,
  NODE_STATE_COORD_GOT_NACK,
  NODE_STATE_WAITING_FOR_COORDINATOR,
};

/************************ types ********************************/
struct node_data {
  /** node id */
  int id;
  /** current node state */
  enum node_state state;
  /** For each node, nonzero if we're waiting on a response from that 
   * node */
  char waiting[MAX_ACCEPTORS];
  /** For each node, nonzero if we believe that node has failed */
  char failed[MAX_ACCEPTORS];
  /** In state NODE_STATE_WAITING_FOR_COORDINATOR, the current
   * coordinator */
  int coord;
  /** who we believe is the current leader, or -1 if there is none */
  int leader;
};

struct mmm_do_propose {
  struct worker_msg base;
};

struct mmm_propose {
  struct worker_msg base;
  /** ID of sender */
  int src;
};

struct mmm_resp {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** 0 for NACK, 1 for ACK */
  int ack;
};

struct mmm_commit_or_abort {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** new leader, or -1 to abort */
  int leader;
};

/************************ globals ********************************/
/** enable debugging spew */ 
static int g_verbose;

/** acceptor nodes */
static struct worker *g_nodes[MAX_ACCEPTORS];

/** acceptor node data */
static struct node_data g_node_data[MAX_ACCEPTORS];

/** number of acceptors */
static int g_num_nodes;

static sem_t g_sem_accept_leader;

/** True if acceptors are running */
static int g_start;

/** Lock that protects g_start */
pthread_mutex_t g_start_lock;

/** Condition variable associated with g_start */
pthread_cond_t g_start_cond;

/************************ functions ********************************/

static int node_is_dead(struct node_data *me, int id)
{
  if (id < 0)
    return 1;
  return me->failed[id];
}

static int tpc_handle_msg(struct worker_msg *m, void *v)
{
  struct node_data *me = (struct node_data*)v;
  struct mmm_propose *mprop;
  struct mmm_resp *mresp;
  struct mmm_commit_or_abort *mca;
  int i;

  pthread_mutex_lock(&g_start_lock);
  while (g_start == 0)
    pthread_cond_wait(&g_start_cond, &g_start_lock);
  pthread_mutex_unlock(&g_start_lock);

  switch (m->ty) {
  case MMM_DO_PROPOSE:
    if ((me->state != NODE_STATE_INIT) ||
        !node_is_dead(me, me->leader)) {
      if (g_verbose)
        fprintf(stderr, "%d: ignoring do_propose\n", me->id);
      break;
    }
    memset(me->waiting, 0, sizeof(me->waiting));
    for (i = 0; i < g_num_nodes; ++i) {
      struct mmm_propose *m;

      if (i == me->id)
        continue;
      m = static_cast<mmm_propose*>(xcalloc(1, sizeof(struct mmm_propose)));
      // learn from
      // http://stackoverflow.com/questions/2474091/why-can-i-set-an-anonymous-enum-equal-to-another-in-c-but-not-c
      m->base.ty = static_cast<worker_msg_ty>(MMM_PROPOSE);
      m->src = me->id;
      if (worker_sendmsg_or_free(g_nodes[i], m)) {
        fprintf(stderr, "%d: declaring node %d as failed\n", me->id, i);
        me->failed[i] = 1;
      }
      else {
        if (g_verbose)
          fprintf(stderr, "%d: sent MMM_PROPOSE to node %d\n", me->id, i);
        me->waiting[i] = 1;
      }
    }
    me->state = NODE_STATE_COORD;
    break;
  case MMM_PROPOSE:
    mprop = (struct mmm_propose *)m;
    if ((!node_is_dead(me, me->leader)) ||
        (me->state != NODE_STATE_INIT)) {
      struct mmm_resp *m;

      m = static_cast<mmm_resp*>(xcalloc(1, sizeof(struct mmm_resp)));
      m->base.ty = static_cast<worker_msg_ty>(MMM_RESP);
      m->src = me->id;
      m->ack = 0;
      if (worker_sendmsg_or_free(g_nodes[mprop->src], m)) {
        fprintf(stderr, "%d: 2PC coordinator %d crashed: "
            "we're screwed\n", me->id, mprop->src);
        abort();
      }
      if (g_verbose)
        fprintf(stderr, "%d: rejected proposal from %d\n",
            me->id, mprop->src);
    }
    else {
      struct mmm_resp *m;

      m = static_cast<mmm_resp*>(xcalloc(1, sizeof(struct mmm_resp)));
      m->base.ty = static_cast<worker_msg_ty>(MMM_RESP);
      m->src = me->id;
      m->ack = 1;
      if (worker_sendmsg_or_free(g_nodes[mprop->src], m)) {
        fprintf(stderr, "%d: 2PC coordinator %d crashed: "
            "we're screwed\n", me->id, mprop->src);
        abort();
      }
      if (g_verbose)
        fprintf(stderr, "%d: accepted proposal from %d\n",
            me->id, mprop->src);
      me->state = NODE_STATE_WAITING_FOR_COORDINATOR;
      me->coord = mprop->src;
    }
    break;
  case MMM_RESP:
    if (!((me->state == NODE_STATE_COORD) ||
          (me->state == NODE_STATE_COORD_GOT_NACK))) {
      fprintf(stderr, "%d: got response message, but we are "
          "not coordinating. %d\n", me->id, m->ty);
      break;
    }
    mresp = (struct mmm_resp *)m;
    me->waiting[mresp->src] = 0;
    if (g_verbose)
      fprintf(stderr, "%d: got %s from %d\n",
          me->id, (mresp->ack ? "ACK" : "NACK"),  mresp->src);
    if (mresp->ack == 0)
      me->state = NODE_STATE_COORD_GOT_NACK;
    for (i = 0; i < g_num_nodes; ++i) {
      if (me->waiting[i] != 0)
        break;
    }
    if (i != g_num_nodes) {
      /* still waiting for some responses */
      break;
    }
    /* send commit/abort message to all */
    if (g_verbose)
      fprintf(stderr, "%d: sending %s to everyone\n", me->id,
          ((me->state == NODE_STATE_COORD_GOT_NACK) ? "ABORT" : "COMMIT"));
    for (i = 0; i < g_num_nodes; ++i) {
      struct mmm_commit_or_abort *m;

      if (i == me->id)
        continue;
      m = static_cast<mmm_commit_or_abort*>(xcalloc(1, sizeof(struct mmm_commit_or_abort)));
      m->base.ty = static_cast<worker_msg_ty>(MMM_COMMIT_OR_ABORT);
      m->src = me->id;
      m->leader = (me->state == NODE_STATE_COORD_GOT_NACK) ? -1 : me->id;
      if (worker_sendmsg_or_free(g_nodes[i], m)) {
        fprintf(stderr, "%d: declaring node %d as failed\n", me->id, i);
        me->failed[i] = 1;
      }
      else {
        me->waiting[i] = 1;
      }
    }
    if (me->state == NODE_STATE_COORD) {
      me->leader = me->id;
      sem_post(&g_sem_accept_leader);
    }
    else {
      int delay;
      struct mmm_do_propose *m;

      /* Wait a bit, then try to propose ourselves as the
       * leader again. */
      m = static_cast<mmm_do_propose*>(xcalloc(1, sizeof(struct mmm_do_propose)));
      m->base.ty = static_cast<worker_msg_ty>(MMM_DO_PROPOSE);
      delay = random() % 100;
      worker_sendmsg_deferred_ms(g_nodes[me->id], m, delay);
    }
    me->state = NODE_STATE_INIT;
    break;
  case MMM_COMMIT_OR_ABORT:
    mca = (struct mmm_commit_or_abort*)m;
    if (me->state != NODE_STATE_WAITING_FOR_COORDINATOR) {
      if (mca->leader == -1) {
        if (g_verbose)
          fprintf(stderr, "%d: received ABORT from %d\n",
              me->id, mca->src);
        break;
      }
      fprintf(stderr, "%d: received MMM_COMMIT_OR_ABORT, but "
          "we are not waiting for a coordinator.\n", me->id);
      break;
    }
    if (me->coord != mca->src) {
      if (mca->leader != -1) {
        fprintf(stderr, "%d: received MMM_COMMIT_OR_ABORT, but "
            "we are waiting for node %d, not node %d\n",
            me->id, me->coord, mca->src);
      }
      break;
    }
    if (mca->leader != -1) {
      me->leader = mca->leader;
      sem_post(&g_sem_accept_leader);
    }
    me->state = NODE_STATE_INIT;
    me->coord = -1;
    break;
  default:
    fprintf(stderr, "%d: invalid message type %d\n", me->id, m->ty);
    abort();
    break;
  }
  return 0;
}

static void send_do_propose(int node_id)
{
  struct mmm_do_propose *m;

  m = static_cast<mmm_do_propose*>(xcalloc(1, sizeof(struct mmm_do_propose)));
  m->base.ty = static_cast<worker_msg_ty>(MMM_DO_PROPOSE);
  worker_sendmsg_or_free(g_nodes[node_id], m);
  //return worker_sendmsg_or_free(g_nodes[node_id], m);
}

static int check_leaders(void)
{
  int i, leader, conflict;

  leader = -1;
  conflict = 0;
  for (i = 0; i < g_num_nodes; ++i) {
    if (leader == -1)
      leader = g_node_data[i].leader;
    else {
      if (leader != g_node_data[i].leader) {
        conflict = 1;
      }
    }
  }
  if (!conflict) {
    printf("successfully elected node %d as leader.\n", leader);
    return 0;
  }
  for (i = 0; i < g_num_nodes; ++i) {
    fprintf(stderr, "%d: leader = %d\n", i, g_node_data[i].leader);
  }
  return 1;
}

static int run_tpc(int duelling_proposers)
{
  int i;

  if (sem_init(&g_sem_accept_leader, 0, 0))
    abort();
  if (pthread_mutex_init(&g_start_lock, 0))
    abort();
  if (pthread_cond_init(&g_start_cond, 0))
    abort();
  g_start = 0;
  memset(g_nodes, 0, sizeof(g_nodes));
  memset(g_node_data, 0, sizeof(g_node_data));
  for (i = 0; i < g_num_nodes;  ++i) {
    char name[WORKER_NAME_MAX];

    snprintf(name, WORKER_NAME_MAX, "node_%3d", i);
    g_node_data[i].id = i;
    g_node_data[i].leader = -1;
    g_node_data[i].coord = -1;
    g_nodes[i] = worker_start(name, tpc_handle_msg,
        NULL, &g_node_data[i]);
    if (!g_nodes[i]) {
      fprintf(stderr, "failed to allocate node %d\n", i);
      abort();
    }
  }
  if (g_num_nodes < 3)
    abort();
  send_do_propose(2);
  if (duelling_proposers) {
    send_do_propose(0);
    send_do_propose(1);
  }
  /* start acceptors */
  pthread_mutex_lock(&g_start_lock);
  g_start = 1;
  pthread_cond_broadcast(&g_start_cond);
  pthread_mutex_unlock(&g_start_lock);
  /* wait for consensus */
  for (i = 0; i < g_num_nodes; ++i) {
    TEMP_FAILURE_RETRY(sem_wait(&g_sem_accept_leader));
  }
  /* cleanup */
  for (i = 0; i < g_num_nodes; ++i) {
    worker_stop(g_nodes[i]);
  }
  for (i = 0; i < g_num_nodes; ++i) {
    worker_join(g_nodes[i]);
  }
  pthread_cond_destroy(&g_start_cond);
  g_start = 0;
  pthread_mutex_destroy(&g_start_lock);
  sem_destroy(&g_sem_accept_leader);

  return check_leaders();
}

static void usage(const char *argv0, int retcode)
{
  fprintf(stderr, "%s: a consensus protocol demo.\n\
      options:\n\
      -h:                     this help message\n\
      -n <num-nodes>          set the number of acceptors (default: %d)\n\
      -v                      enable verbose debug messages\n\
      ", argv0, DEFAULT_NUM_NODES);
  exit(retcode);
}

static void parse_argv(int argc, char **argv)
{
  int c;

  g_num_nodes = DEFAULT_NUM_NODES;
  g_verbose = 0;

  while ((c = getopt(argc, argv, "hn:v")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0], EXIT_SUCCESS);
      break;
    case 'n':
      g_num_nodes = atoi(optarg);
      break;
    case 'v':
      g_verbose = 1;
      break;
    default:
      fprintf(stderr, "failed to parse argument.\n\n");
      usage(argv[0], EXIT_FAILURE);
      break;
    }
  }
  if (g_num_nodes < 3) {
    fprintf(stderr, "num_nodes = %d, but we can't run with "
        "num_nodes < 3.\n\n", g_num_nodes);
    usage(argv[0], EXIT_FAILURE);
  }
}

int main(int argc, char **argv)
{
  int ret;

  parse_argv(argc, argv);

  /* seed random number generator */
  srandom(time(NULL));

  worker_init();

  printf("testing single-proposer case...\n");
  ret = run_tpc(0);
  if (ret) {
    fprintf(stderr, "run_tpc(0) failed with error code %d\n", ret);
    return EXIT_FAILURE;
  }

  printf("\ntesting multi-proposer case...\n");
  ret = run_tpc(1);
  if (ret) {
    fprintf(stderr, "run_tpc(1) failed with error code %d\n", ret);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
