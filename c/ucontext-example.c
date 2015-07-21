// gcc ucontext-example.c -o ucontext-example --std=gnu99
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>

#define STACK_SIZE 1024
#define MAX_COROS 8

int current = 0;  // 当前协程

typedef struct coro_t {
  int used;
  ucontext_t context;
  char stack[STACK_SIZE];
  void* (*func)(void *arg);
  void *arg;
  void *exit_status;
} coro_t;

static coro_t coro_slots[MAX_COROS];  // 用以维护当前线程/进程的所有协程

// 从slots里调度一个协程执行
void coro_schedule(void) {
  int i;
  for (i = (current + 1) % MAX_COROS; i != current; i = (i + 1) % MAX_COROS)
    if (coro_slots[i].used) break;  // 找到slots里current后面的第一个active的协程

  int prev = current;
  current = i;
  printf("coro_schedule stop coro-%d, and running coro-%d\n", prev, current);
  swapcontext(&coro_slots[prev].context, &coro_slots[current].context);  // 保存旧状态，切换到新状态
}

void coro_exit(void) {
  void *exit_status = coro_slots[current].func(coro_slots[current].arg);
  coro_slots[current].exit_status = exit_status;
  coro_slots[current].used = 0;
  coro_schedule();
}

int coro_create(void* (*start_routine)(void*), void *arg) {
  static int last_used = 0;
  int i, tid;

  for (i = (last_used + 1) % MAX_COROS; i != last_used; i = (i + 1) % MAX_COROS)
    if (!coro_slots[i].used) break;
  if (i == last_used)
    return -1;
  last_used = i;
  tid = i;

  /*
  typedef struct ucontext {
    struct ucontext *uc_link;       // 如果当前context终结，将恢复执行uc_link指向的context
    sigset_t         uc_sigmask;    // 当前context阻止的信号集合，sigprocmask(2)
    stack_t          uc_stack;      // 当前context使用的栈，sigaltstack(2)
    mcontext_t       uc_mcontext;   // machine-specific，包含调用者线程的寄存器等
    ...
  } ucontext_t;
  */
  // 原型：int getcontext(ucontext_t *ucp);
  getcontext(&coro_slots[tid].context);  // 将当前的、active的context保存到ucp中
  coro_slots[tid].context.uc_stack.ss_sp = coro_slots[tid].stack;  // 调用makecontext之前必须设定uc_stack，和uc_link
  coro_slots[tid].context.uc_stack.ss_size = sizeof(coro_slots[tid].stack);
  coro_slots[tid].context.uc_link = &coro_slots[0].context;

  coro_slots[i].used = 1;
  coro_slots[i].func = start_routine;
  coro_slots[i].arg = arg;
  coro_slots[i].exit_status = 0;
  // 原型：makecontext(ucontext_t* ucp, void (*func)(), int argc, ...);
  makecontext(&coro_slots[i].context, coro_exit, 0);  // 修改getcontext获取的ucp，如果ucp后面通过setcontext/swapcontext被激活，则函数func将被调用
  return 0;
}

void* coro_func(void *arg) {
  printf("running coro-%d coro_func(%s)\n", current, (char*)arg);
  coro_schedule();
}

int main(int argc, char *argv[]) {
  coro_create(coro_func, "coro1 args");  // 创建子协程，但仍未触发调用
  coro_create(coro_func, "coro2 args");

  coro_schedule();

  return 0;
}

/*
 * coro_schedule stop coro-0, and running coro-1
 * running coro-1 coro_func(coro1 args)
 * coro_schedule stop coro-1, and running coro-2
 * running coro-2 coro_func(coro2 args)
 * coro_schedule stop coro-2, and running coro-2
 * coro_schedule stop coro-2, and running coro-2
*/
