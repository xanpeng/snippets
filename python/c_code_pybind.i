// 使用方式：
// swig -python c_code_pybind.i
// gcc -c $(python_include) c_code_pybind_wrap.c -o c_code_pybind_wrap.o
//      // python_include=$(shell python-config --cflags)
// ld -shared c_code_pybind_wrap.o -o _c_code_pybind_wrap.so

%module c_port_to_python_example

%{
#define SWIG_FILE_WITH_INIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-header.h"
%}

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

// 这些函数不需要做额外处理，swig生成的函数就满足需求
%rename example_c_func0 pythonic_name_func0;

extern int example_c_func0(uint32_t arg0, uint16_t arg1, bool arg2);

// 这些函数需要作额外的处理
// C函数的签名：extern int example_c_func1(uint32_t arg0, uint16_t arg1, struct ExampleStruct** arg3);
%rename example_c_func1 pythonic_name_func1;
%inline %{
PyObject* example_c_func1_wrap(uint32_t arg0, uint16_t arg1) {
  struct ExampleStruct *temp;
  int cnt = example_c_func1(arg0, arg1, &temp);
  PyObject* result = PyList_New(cnt);
  for (int i = 0; i < cnt; ++i) {
    PyList_SetItem(result, i, Py_BuildValue("II", temp[i].some_int_val0, temp[i].some_int_val1));
  }
  return result;
}%}

