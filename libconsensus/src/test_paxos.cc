/*
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

#include "limits.h"
#include "util.h"
#include "worker.h"

#include <muduo/base/Logging.h>
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
  MMM_DO_TIMEOUT,
  MMM_PROPOSE = 3000,
  MMM_REJECT,
  MMM_ACCEPT,
  MMM_COMMIT,
  MMM_COMMIT_RESP,
};

enum node_state {
  NODE_STATE_INIT = 0,
  NODE_STATE_PROPOSING,
  NODE_STATE_COMMITTING,
};

enum remote_state {
  REMOTE_STATE_NO_RESP = 0,
  REMOTE_STATE_FAILED,
  REMOTE_STATE_ACCEPTED,
  REMOTE_STATE_REJECTED,
};

/************************ types ********************************/
struct node_data {
  /** node id */
  int id;
  /** current node state */
  enum node_state state;
  /** Perceived state of remote nodes */
  uint8_t remotes[MAX_ACCEPTORS];
  /** The highest paxos sequence number we've seen so far */
  uint64_t seen_pseq;
  /** In NODE_STATE_PROPOSING, the paxos sequence number we are currently
   * proposing */
  uint64_t prop_pseq;
  /** The highest leader we've accept yet, or -1 if none has been proposed */
  int prop_leader;
  /** who we believe is the current leader, or -1 if there is none */
  int leader;
};

struct mmm_do_propose {
  struct worker_msg base;
};

struct mmm_do_timeout {
  struct worker_msg base;
  uint64_t prop_pseq;
};

struct mmm_propose {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** The value being proposed */
  int prop_leader;
  /** Paxos sequence ID of this propsal */
  uint64_t prop_pseq;
};

struct mmm_resp {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** Value of the highest-numbered proposal that this node has already
   * accepted */
  int leader;
};

struct mmm_reject {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** Sequence number of the highest proposal we've seen */
  uint64_t seen_pseq;
};

struct mmm_accept {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** The leader that we are accepting */
  int prop_leader;
  /** The sequence number of the coordinator from which we heard this
   * prop_leader */
  uint64_t seen_pseq;
};

struct mmm_commit {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** The value to commit */
  int prop_leader;
  /** Paxos sequence ID of this propsal */
  uint64_t prop_pseq;
};

struct mmm_commit_resp {
  struct worker_msg base;
  /** ID of sender */
  int src;
  /** nonzero if this is an affirmative response */
  int ack;
  /** Paxos sequence ID of the commit propsal we're responding to */
  uint64_t prop_pseq;
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
static const char *msg_type_to_str(int ty)
{
  switch (ty) {
  case MMM_DO_PROPOSE:
    return "MMM_DO_PROPOSE";
  case MMM_DO_TIMEOUT:
    return "MMM_DO_TIMEOUT";
  case MMM_PROPOSE:
    return "MMM_PROPOSE";
  case MMM_REJECT:
    return "MMM_REJECT";
  case MMM_ACCEPT:
    return "MMM_ACCEPT";
  case MMM_COMMIT:
    return "MMM_COMMIT";
  case MMM_COMMIT_RESP:
    return "MMM_COMMIT_RESP";
  default:
    return "(unknown)";
  }
}

static int node_is_dead(struct node_data *me, int id)
{
  if (id < 0)
    return 1;
  return me->remotes[id] == REMOTE_STATE_FAILED;
}

static uint64_t outbid(uint64_t pseq, int id)
{
  uint64_t tmp;

  tmp = pseq >> 8;
  tmp++;
  tmp <<= 8;
  if (id > 0xff)
    abort();
  return tmp | id;
}

static void reset_remotes(uint8_t *remotes)
{
  int i;

  for (i = 0; i < g_num_nodes; ++i) {
    switch (remotes[i]) {
    case REMOTE_STATE_ACCEPTED:
    case REMOTE_STATE_REJECTED:
      remotes[i] = REMOTE_STATE_NO_RESP;
    }
  }
}

static int check_acceptor_resp(const struct node_data *me, int ty, int src)
{
  if (me->state != NODE_STATE_PROPOSING) {
    if (g_verbose) {
      LOG_DEBUG << me->id << ": got " << msg_type_to_str(ty) << " message from "
        << src << ", but we are not proposing.";
    }
    return 1;
  }
  if (me->remotes[src] != REMOTE_STATE_NO_RESP) {
    LOG_DEBUG << me->id << ": got " << msg_type_to_str(ty) << " message from "
      << src << ", but remote_state for that node is already " << me->remotes[src];
    return 1;
  }
  return 0;
}

static int have_majority_of_acceptors(const struct node_data *me)
{
  int i, num_acc;

  num_acc = 0;
  for (i = 0; i < g_num_nodes; ++i) {
    if (me->remotes[i] == REMOTE_STATE_ACCEPTED)
      num_acc++;
  }
  return (num_acc > (g_num_nodes / 2));
}

static int cannot_have_majority_of_acceptors(const struct node_data *me)
{
  int i, num_rej;

  num_rej = 0;
  for (i = 0; i < g_num_nodes; ++i) {
    if (me->remotes[i] == REMOTE_STATE_REJECTED)
      num_rej++;
  }
  return (num_rej >= (g_num_nodes / 2));
}

static void send_delayed_do_propose(struct node_data *me)
{
  struct mmm_do_propose *mdop;
  int delay;

  mdop = static_cast<mmm_do_propose*>(xcalloc(1, sizeof(struct mmm_do_propose)));
  mdop->base.ty = static_cast<worker_msg_ty>(MMM_DO_PROPOSE);
  delay = random() % 100;
  worker_sendmsg_deferred_ms(g_nodes[me->id], mdop, delay);
}

static void send_do_propose(int node_id)
{
  struct mmm_do_propose *mdop;

  mdop = static_cast<mmm_do_propose*>(xcalloc(1, sizeof(struct mmm_do_propose)));
  mdop->base.ty = static_cast<worker_msg_ty>(MMM_DO_PROPOSE);
  worker_sendmsg_or_free(g_nodes[node_id], mdop);
}

static void send_commits(struct node_data *me)
{
  int i;

  for (i = 0; i < g_num_nodes; ++i) {
    struct mmm_commit *mcom;

    if (i == me->id)
      continue;
    mcom = static_cast<mmm_commit*>(xcalloc(1, sizeof(struct mmm_commit)));
    mcom->base.ty = static_cast<worker_msg_ty>(MMM_COMMIT);
    mcom->src = me->id;
    mcom->prop_leader = me->prop_leader;
    mcom->prop_pseq = me->prop_pseq;
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": sending COMMIT (prop_leader=" << mcom->prop_leader <<
        ", prop_pseq=0x" << std::hex << mcom->prop_pseq << " message to " << i;
    }
    if (worker_sendmsg_or_free(g_nodes[i], mcom))
    {
      LOG_DEBUG << me->id << ": declaring node " << i << " as failed";
      me->remotes[i] = REMOTE_STATE_FAILED;
    }
    else
    {
      me->remotes[i] = REMOTE_STATE_NO_RESP;
    }
  }
}

static int check_commit_resp(const struct node_data *me,
    const struct mmm_commit_resp *mresp)
{
  if (me->state != NODE_STATE_COMMITTING)
  {
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": got MMM_COMMIT_RESP from " << mresp->src
        << ", but we are not commiting anymore.";
    }
    return 1;
  }
  if (mresp->prop_pseq != me->prop_pseq)
  {
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": got MMM_COMMIT_RESP from " << mresp->src
        << ", but mresp->prop_pseq is 0x" << std::hex << mresp->prop_pseq
        << ", not 0x" << me->prop_pseq;
    }
    return 1;
  }
  return 0;
}

static void accept_leader(struct node_data *me, int leader)
{
  me->prop_leader = leader;
  me->leader = leader;
  sem_post(&g_sem_accept_leader);
  me->state = NODE_STATE_INIT;
  if (g_verbose)
  {
    LOG_DEBUG << me->id << ": accepted leader " << leader;
  }
}

static void abandon_proposal(struct node_data *me, const char *why)
{
  if (g_verbose)
  {
    LOG_DEBUG << me->id << ": abandoning proposal 0x" <<
      std::hex << me->prop_pseq << " " << why;
  }
  me->prop_pseq = -1;
  me->state = NODE_STATE_INIT;
  reset_remotes(me->remotes);
  /* Wait a bit, then try to propose ourselves as the
   * leader again. */
  send_delayed_do_propose(me);
}

static int paxos_handle_msg(struct worker_msg *m, void *v)
{
  struct node_data *me = (struct node_data*)v;
  struct mmm_propose *mprop;
  struct mmm_do_timeout *mtimeo;
  struct mmm_accept *macc; 
  struct mmm_reject *mrej; 
  struct mmm_commit *mcom; 
  struct mmm_commit_resp *mresp;
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
      {
        LOG_DEBUG << me->id << ": ignoring do_propose";
      }
      break;
    }
    reset_remotes(me->remotes);
    me->prop_pseq  = me->seen_pseq = outbid(me->seen_pseq, me->id);
    me->prop_leader = me->id;
    for (i = 0; i < g_num_nodes; ++i) {
      if (i == me->id)
        continue;
      mprop = static_cast<mmm_propose*>(xcalloc(1, sizeof(struct mmm_propose)));
      mprop->base.ty = static_cast<worker_msg_ty>(MMM_PROPOSE);
      mprop->src = me->id;
      mprop->prop_leader = me->id;
      mprop->prop_pseq = me->prop_pseq;
      if (worker_sendmsg_or_free(g_nodes[i], mprop))
      {
        LOG_DEBUG << me->id << ": declaring node " << i << " as failed";
        me->remotes[i] = REMOTE_STATE_FAILED;
      }
      else {
        if (g_verbose)
        {
          LOG_DEBUG << me->id << ": sent MMM_PROPOSE to node " << i;
        }
        me->remotes[i] = REMOTE_STATE_NO_RESP;
      }
    }
    mtimeo = static_cast<mmm_do_timeout*>(xcalloc(1, sizeof(struct mmm_do_timeout)));
    mtimeo->base.ty = static_cast<worker_msg_ty>(MMM_DO_TIMEOUT);
    mtimeo->prop_pseq = me->prop_pseq;
    worker_sendmsg_deferred_ms(g_nodes[me->id], mtimeo, 1000);
    me->state = NODE_STATE_PROPOSING;
    break;
  case MMM_DO_TIMEOUT:
    mtimeo = (struct mmm_do_timeout*)m;
    if ((mtimeo->prop_pseq != me->prop_pseq) ||
        (me->state != NODE_STATE_PROPOSING)) {
      if (g_verbose)
      {
        LOG_DEBUG << me->id << ": ignoring timeout for defunct pseq 0x"
          << std::hex << mtimeo->prop_pseq;
      }
      break;
    }
    abandon_proposal(me, "because of timeout");
    break;
  case MMM_PROPOSE:
    mprop = (struct mmm_propose *)m;
    if ((!node_is_dead(me, me->leader)) || 
        (mprop->prop_pseq <= me->seen_pseq)) {
      struct mmm_reject *mrej;

      mrej = static_cast<mmm_reject*>(xcalloc(1, sizeof(struct mmm_reject)));
      mrej->base.ty = static_cast<worker_msg_ty>(MMM_REJECT);
      mrej->src = me->id;
      mrej->seen_pseq = me->seen_pseq;
      if (worker_sendmsg_or_free(g_nodes[mprop->src], mrej)) {
        me->remotes[mprop->src] = REMOTE_STATE_FAILED;
      }
      if (g_verbose)
      {
        LOG_DEBUG << me->id << ": rejected proposal (mprop->leader=" << mprop->prop_leader
          << ", mprop->seen_pseq=0x" << std::hex << mprop->prop_pseq << " from " << mprop->src
          << "(me->leader=" << me->leader << ", me->seen_pseq=0x" << std::hex << me->seen_pseq
          << ")";
      }
    }
    else {
      macc = static_cast<mmm_accept*>(xcalloc(1, sizeof(struct mmm_accept)));
      if (me->prop_leader == -1) {
        macc->prop_leader = -1;
        macc->seen_pseq = 0;
      }
      else {
        macc->prop_leader = me->prop_leader;
        macc->seen_pseq = me->seen_pseq;
      }
      me->prop_leader = mprop->prop_leader;
      me->seen_pseq = mprop->prop_pseq;
      macc->base.ty = static_cast<worker_msg_ty>(MMM_ACCEPT);
      macc->src = me->id;
      if (worker_sendmsg_or_free(g_nodes[mprop->src], macc)) {
        me->remotes[mprop->src] = REMOTE_STATE_FAILED;
      }
      if (me->state != NODE_STATE_INIT) {
        if (g_verbose)
        {
          LOG_DEBUG << me->id << ": giving up on proposing because node " << mprop->src
            << " had a proposal with a higher sequence number";
        }
        me->state = NODE_STATE_INIT;
      }
    }
    break;
  case MMM_ACCEPT:
    macc = (struct mmm_accept *)m;
    if (check_acceptor_resp(me, MMM_ACCEPT, macc->src))
      break;
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": got MMM_ACCEPT(prop_leader=" << macc->prop_leader
        << ") from " << macc->src;
    }
    me->remotes[macc->src] = REMOTE_STATE_ACCEPTED;
    if (!have_majority_of_acceptors(me))
      break;
    send_commits(me);
    me->state = NODE_STATE_COMMITTING;
    break;
  case MMM_REJECT:
    mrej = (struct mmm_reject *)m;
    if (check_acceptor_resp(me, MMM_REJECT, mrej->src))
      break;
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": got MMM_REJECT(seen_pseq=0x" << std::hex << mrej->seen_pseq
        << ") from " << mrej->src;
    }
    me->remotes[mrej->src] = REMOTE_STATE_REJECTED;
    if (cannot_have_majority_of_acceptors(me))
      abandon_proposal(me, " because of too much rejection");
    break;
  case MMM_COMMIT:
    mcom = (struct mmm_commit*)m;
    mresp = static_cast<mmm_commit_resp*>(xcalloc(1, sizeof(struct mmm_commit_resp)));
    mresp->base.ty = static_cast<worker_msg_ty>(MMM_COMMIT_RESP);
    mresp->src = me->id;
    mresp->prop_pseq = mcom->prop_pseq;
    if ((me->state != NODE_STATE_INIT) ||
        (mcom->prop_leader != me->prop_leader) ||
        (mcom->prop_pseq < me->prop_pseq)) {
      mresp->ack = 0;
    }
    else {
      mresp->ack = 1;
      accept_leader(me, mcom->prop_leader);
    }
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": sending commit " << (mresp->ack ? "ACK" : "NACK")
        << " back to " << mcom->src;
    }
    if (worker_sendmsg_or_free(g_nodes[mcom->src], mresp)) {
      me->remotes[mcom->src] = REMOTE_STATE_FAILED;
    }
    break;
  case MMM_COMMIT_RESP:
    mresp = (struct mmm_commit_resp *)m;
    if (check_commit_resp(me, mresp))
      break;
    if (!mresp->ack) {
      me->remotes[mresp->src] = REMOTE_STATE_REJECTED;
      break;
    }
    me->remotes[mresp->src] = REMOTE_STATE_ACCEPTED;
    if (!have_majority_of_acceptors(me))
      break;
    accept_leader(me, me->prop_leader);
    if (g_verbose)
    {
      LOG_DEBUG << me->id << ": considering protocol terminated";
    }
    break;
  default:
    LOG_DEBUG << me->id << ": invalid message type " << m->ty;
    abort();
    break;
  }
  return 0;
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
  for (i = 0; i < g_num_nodes; ++i)
  {
    LOG_DEBUG << i << ": leader=" << g_node_data[i].leader;
  }
  return 1;
}

static int run_paxos(int duelling_proposers)
{
  int i;
  struct timespec ts;

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
    g_node_data[i].state = NODE_STATE_INIT;
    reset_remotes(g_node_data[i].remotes);
    g_node_data[i].seen_pseq = 0;
    g_node_data[i].prop_pseq = 0;
    g_node_data[i].prop_leader = -1;
    g_node_data[i].leader = -1;
    g_nodes[i] = worker_start(name, paxos_handle_msg,
        NULL, &g_node_data[i]);
    if (!g_nodes[i])
    {
      LOG_ERROR << "failed to allocate node " << i;
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
  /* Wait for consensus.
   * We only actually need more than half the nodes.  However, to make
   * debugging a little nicer, we'll wait 10 seconds for all the remaining
   * nodes rather than exiting immediately after we get half. */
  for (i = 0; i < 1 + (g_num_nodes / 2); ++i) {
    TEMP_FAILURE_RETRY(sem_wait(&g_sem_accept_leader));
  }
  if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    abort();
  ts.tv_sec += 10;
  for (; i < g_num_nodes; ++i) {
    TEMP_FAILURE_RETRY(sem_timedwait(&g_sem_accept_leader, &ts));
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
  LOG_ERROR << argv0 << ": a consensus protocol demo.";
  LOG_ERROR << "options:";
  LOG_ERROR << "-h:             this help message";
  LOG_ERROR << "-n <num-nodes>  set the number of acceptors" 
    << " (default: " << DEFAULT_NUM_NODES << ")";
  LOG_ERROR << "-v              enabel verbose debug messages";
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
      LOG_ERROR << "failed to parse argument";
      usage(argv[0], EXIT_FAILURE);
      break;
    }
  }
  if (g_num_nodes < 3)
  {
    LOG_ERROR << "num_nodes=" << g_num_nodes
      << ", but we can't run with num_nodes<3";
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

  muduo::Logger::setLogLevel(muduo::Logger::DEBUG);
  LOG_INFO << "testing single-proposer case...";
  ret = run_paxos(0);
  if (ret) {
    LOG_ERROR << "run_paxos(0) failed with error code " << ret;
    return EXIT_FAILURE;
  }

  LOG_INFO << "testing multi-proposer case...";
  ret = run_paxos(1);
  if (ret) {
    LOG_ERROR << "run_paxos(1) failed with error code " << ret;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
