//
// A Thread abstraction from Ceph, pthread is used.
//
#include <pthread.h>

class Thread {
private:
  pthread_t thread_id;

public:
  Thread(const Thread& other);
  const Thread& operator=(const Thread& other);

  Thread();
  virtual ~Thread();

protected:
  virtual void *entry() = 0;

private:
  static void *_entry_func(void *arg);

public:
  const pthread_t &get_thread_id();
  bool is_started();
  bool am_self();
  int kill(int signal);
  int try_create(size_t stacksize);
  void create(size_t stacksize = 0); // wrap try_create
  int join(void **prval = 0);
  int detach();
};

Thread::Thread() : thread_id(0) {}
Thread::~Thread() {}

void* Thread::_entry_func(void* arg) {
  void* r = ((Thread*)arg)->entry();
  return r;
}

const pthread_t &Thread::get_thread_id() { return thread_id; }
bool Thread::is_started() { return thread_id != 0; }
bool Thread::am_self() { return (pthread_self() == thread_id); }

int Thread::kill(int signal) {
  if (thread_id) return pthread_kill(thread_id, signal);
  else return -EINVAL;
}

int Thread::try_create(size_t stacksize) {
  pthread_attr_t *thread_attr = NULL;
  stacksize &= CEPH_PAGE_MASK;  // must be multiple of page
  if (stacksize) {
    thread_attr = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
    if (!thread_attr) return -ENOMEM;
    pthread_attr_init(thread_attr);
    pthread_attr_setstacksize(thread_attr, stacksize);
  }

  int r;

  // The child thread will inherit our signal mask.  Set our signal mask to
  // the set of signals we want to block.  (It's ok to block signals more
  // signals than usual for a little while-- they will just be delivered to
  // another thread or delieverd to this thread later.)
  sigset_t old_sigset;
  if (g_code_env == CODE_ENVIRONMENT_LIBRARY) {
    block_signals(NULL, &old_sigset);
  }
  else {
    int to_block[] = { SIGPIPE , 0 };
    block_signals(to_block, &old_sigset);
  }
  r = pthread_create(&thread_id, thread_attr, _entry_func, (void*)this);
  restore_sigset(&old_sigset);

  if (thread_attr)
    free(thread_attr);
  return r;
}

int Thread::join(void **prval) {
  if (thread_id == 0) {
    assert("join on thread that was never started" == 0);
    return -EINVAL;
  }

  int status = pthread_join(thread_id, prval);
  assert(status == 0);
  thread_id = 0;
  return status;
}

int Thread::detach() {
  return pthread_detach(thread_id);
}

/* Block the signals in 'siglist'. If siglist == NULL, block all signals. */
void block_signals(const int *siglist, sigset_t *old_sigset) {
  sigset_t sigset;
  if (!siglist) {
    sigfillset(&sigset);
  }
  else {
    int i = 0;
    sigemptyset(&sigset);
    while (siglist[i]) {
      sigaddset(&sigset, siglist[i]);
      ++i;
    }
  }
  int ret = pthread_sigmask(SIG_BLOCK, &sigset, old_sigset);
  assert(ret == 0);
}

void restore_sigset(const sigset_t *old_sigset) {
  int ret = pthread_sigmask(SIG_SETMASK, old_sigset, NULL);
  assert(ret == 0);
}
