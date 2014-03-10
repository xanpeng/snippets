/*
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

#ifndef CONSENSUS_FUN_WORKER_DOT_H
#define CONSENSUS_FUN_WORKER_DOT_H

#include "tree.h"

#include <inttypes.h>
#include <pthread.h>
#include <sys/time.h> /* for struct timespec */

#define WORKER_NAME_MAX 16

struct worker;
struct worker_msg;

typedef int (*worker_fn_t)(struct worker_msg *msg, void *data);
typedef void (*worker_shutdown_fn_t)(void *data);

enum worker_msg_ty
{
  WORKER_MSG_SHUTDOWN = 9000,
};

/** A message sent to a worker thread.
*/
struct worker_msg
{
  /** Next message in the list */
  struct worker_msg *next;
  /** Type of message */
  enum worker_msg_ty ty;
  /** If deferred, earliest time at which the recipient will receive this
   * message. */
  struct timespec defer_until;
  /** If deferred, destination worker */
  struct worker *dst;
  /** If deferred, pointers to other entries in the deferred tree */
  RB_ENTRY(worker_msg) defer_entry;
  /** Message-specific data */
  char data[0];
};

/** Initialize the worker subsystem.
 *
 * Must be called before any other worker functions are called.
 */
void worker_init(void);

/** Start a worker
 *
 * @name		The name of this worker. It doesn't have to be unique;
 *			it's mainly for debugging. It will be truncated if it's
 *			longer than WORKER_NAME_MAX - 1 characters.
 * @fn			The worker function to use
 * @sfn			The function to call before shutting down, or NULL
 * @data		Data to pass to the worker
 *
 * @return		A pointer to the worker on success; NULL otherwise
 */
struct worker *worker_start(const char *name, worker_fn_t fn,
    worker_shutdown_fn_t sfn, void *data);

/** Send a message to a worker
 *
 * @worker		the worker
 * @m			The message to send to the worker
 *
 * @return		0 on success; error code otherwise
 */
int worker_sendmsg(struct worker *worker, void *m);

/** Send a message to a worker.  Free the message if it can't be sent.
 *
 * @worker		the worker
 * @m			The message to send to the worker
 *
 * @return		0 on success; error code otherwise
 */
int worker_sendmsg_or_free(struct worker *worker, void *m);

/** Stop a worker
 *
 * @worker		The worker to stop 
 *
 * @return		0 if the shutdown message was sent successfully.
 *			error code otherwise
 */
int worker_stop(struct worker *worker);

/** Wait until the a worker thread ends and reclaim its resources.
 * Usually you will want to call worker_stop on the thread before calling this
 * function.
 *
 * @worker		The worker to join 
 *
 * @return		0 if the worker was running and returned success.
 *			Error code otherwise.
 */
int worker_join(struct worker *worker);

/** Queue a message for delivery some time after 'ts'.
 *
 * @param w		The worker to send to
 * @param m		The message
 * @param ts		The earliest time at which the message can be
 *			delivered.  This is absolute time like what you would
 *			get from clock_gettime(CLOCK_REALTIME, &ts);
 */
void worker_sendmsg_deferred(struct worker *w, void *m,
    const struct timespec *ts);

/** Queue a message for delivery no less than 'ms' milliseconds after now
 *
 * @param w		The worker to send to
 * @param m		The message
 * @param ms		Minimum number of milliseconds to wait
 */
void worker_sendmsg_deferred_ms(struct worker *w, void *m, int ms);

/** Clear all deferred messages.
 *
 * The memory associated with them will also be freed.
 */
void clear_deferred(void);

#endif
