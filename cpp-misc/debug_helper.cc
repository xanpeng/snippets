//
// g++ debug_helper.cc -o debug_helper -std=c++11 -DSIMPLE_DEBUG -DVARARGS_DEBUG -DBEST_DEBUG
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

//
// simple debug output technical
// 
#ifdef SIMPLE_DEBUG
#define simple_debug(x) std::cerr << "[debug] " << x << std::endl;
#else
#define simple_debug(x)
#endif

void test_simple_debug() {
  std::cout << "test_simple_debug()\n";
  simple_debug("hello world");
}

//
// better debug output technical
//
// http://stackoverflow.com/questions/16447951/how-to-forward-variable-number-of-arguments-to-another-function
// http://stackoverflow.com/questions/1657883/variable-number-of-arguments-in-c
// http://stackoverflow.com/questions/205529/c-c-passing-variable-number-of-arguments-around
void varargs_debug_helper(const std::string& format, va_list args) {
  const int size = 127;
  char buff[size+1];
  int len1 = snprintf(buff, size, "[debug] ");
  int len2 = vsnprintf(buff + len1, size - len1, format.c_str(), args);
  buff[len1 + len2] = 0;
  fprintf(stderr, "%s\n", buff);
}

void varargs_debug(const std::string& format, ...) {
#ifdef VARARGS_DEBUG
  va_list args;
  va_start(args, format); // compiler warning
  varargs_debug_helper(format, args);  
  va_end(args);
#else
  (void)format;
#endif
}

void test_varargs_debug() {
  std::cout << "test_varargs_debug()\n";
  int count = 4;
  const char* message = "times";
  varargs_debug("%d %s", count, message);
}

//
// va_list用起来不安全（为什么），
// 有人说可以使用variabdic templates，但这样就不能使用格式化输出了（是这样吗），只能多次调用cout<<，从而多线程输出控制台情景下，各行内容会相互冲乱。
//
// http://binglongx.wordpress.com/2013/10/22/c-variadic-templates-a-type-safe-vartmpl_debugf/
// What we want: vartmpl_debug arbitrary number of arbitrary arguments safely
template<typename... Args> void vartmpl_debug(Args&&... args);
// handles no-argument case
void vartmpl_debug() {}
// base case, terminating recursion
template<typename Arg0> void vartmpl_debug(Arg0&& arg0) {
    std::cout << std::forward<Arg0>(arg0);
}
template<typename Arg0, typename... Args> void vartmpl_debug(Arg0&& arg0, Args&&... rest) {
    // vartmpl_debug the first argument.
    std::cout << std::forward<Arg0>(arg0) << " ";
    // recursion for rest arguments
    vartmpl_debug(std::forward<Args>(rest)...); // this resolves to either myself or vartmpl_debug(arg0)
}

void test_vartemplate_debug() {
  std::cout << "test_vartemplate_debug()\n";
  vartmpl_debug(42, 3.14, "Hello_variadic_templates!\n");
  vartmpl_debug(); // this does nothing
}

//
// best solution: variadic templates + 格式化字符串
//
template<typename... Args>
void best_debug(const std::string& format, Args&&... args) {
#ifdef BEST_DEBUG
  const int size = 127;
  char buff[size+1];                                            
  int len1 = snprintf(buff, size, "[debug] ");                      
  int len2 = snprintf(buff + len1, size - len1, format.c_str(), std::forward<Args>(args)...);      
  buff[len1 + len2] = 0;                                                   
  fprintf(stderr, "%s\n", buff);                                           
#endif
}

void test_best_debug() {
  std::cout << "test_best_debug()\n";
  int count = 4;
  const char* message = "times";
  best_debug("%d %s", count, message);
}

int main() {
  test_simple_debug();
  test_varargs_debug();
  test_vartemplate_debug();
  test_best_debug();

  return 0;
}
