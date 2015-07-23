// gcc lwan-coro-example.c lwan-coro.c -o lwan-coro-example -Wconversion -std=gnu99 -g
#include <stdio.h>
#include "coro-lwan-coro.h"


int coro_func_foo(coro_t *coro) {
  printf("called coro_func_foo\n");
  printf("yield in coro_func_foo\n");
  coro_yield(coro, 0);
  printf("continue in coro_func_foo\n");
  return 0;
}

int coro_func_bar(coro_t *coro) {
  printf("called coro_func_bar\n");
  printf("yield in coro_func_bar\n");
  coro_yield(coro, 0);
  printf("continue in coro_func_bar\n");
  return 0;
}


int main() {
  coro_switcher_t switcher;
  printf("coro_new coro1 and coro2\n");
  coro_t *coro1 = coro_new(&switcher, &coro_func_foo, NULL);
  coro_t *coro2 = coro_new(&switcher, &coro_func_bar, NULL);

  coro_resume(coro1);
  coro_resume(coro2);

  coro_resume(coro1);
  coro_resume(coro2);

  printf("coro_free coro1 and coro2\n");
  coro_free(coro1);
  coro_free(coro2);

  return 0;
}

/*
coro_new coro1 and coro2
called coro_func_foo
yield in coro_func_foo
called coro_func_bar
yield in coro_func_bar
continue in coro_func_foo
continue in coro_func_bar
coro_free coro1 and coro2
*/
