/*
qemu coroutine的代码结构
------------------------
qemu/include/block/coroutine.h，
  #define coroutine_fn
  typedef struct Coroutine Coroutine;
  typedef void coroutine_fn CoroutineEntry(void *opaque);
  Coroutine *qemu_coroutine_create(CoroutineEntry* entry);
  void qemu_coroutine_enter(Coroutine *coroutine, void *opaque);
  void coroutine_fn qmeu_coroutine_yield(void);
  Coroutine* coroutine_fn qemu_coroutine_self(void);
  bool qemu_in_coroutine(void);
  一些CoQueue相关的函数
  一些CoMutex相关的函数
  一些CoRwlock相关的函数
实现位于qemu/qemu-coroutine.c，背后再调用
  qemu/coroutine-ucontext.c: getcontext(3), makecontext(3)等
  qemu/coroutine-gthread.c: 应该用到glib gthread这一用户态线程机制
  qemu/coroutine-win32.c: win32下
  qemu/coroutine-sigaltstack.c: setjmp/longjmp

qemu/include/block/coroutine_int.h : coroutine internals
  struct Coroutine定义;
  Coroutine* qemu_coroutine_new(void);
  void qemu_coroutine_delete(Coroutine *co);
  CoroutineAction qemu_coroutine_switch(Coroutine* from , Coroutine* to, CoroutineAction action);
  void coroutine_fn qemu_co_queue_run_restart(Coroutine* co);

qemu/qemu-coroutine-io.c : coroutine-aware I/O functions
qemu/qemu-coroutine-lock.c : coroutine queues and locks
qemu/qemu-coroutine-sleep.c : coroutine sleep

*/
