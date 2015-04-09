/* goal is to expose cos_func to python */
%module cos_module
%{
/* the resulting c file should be built as a python extension */
#define SWIG_FILE_WITH_INIT
/* includes the header in the wrapper code */
#include "cos_module.h"
%}

/* parse the header file to generate wrappers */
%include "cos_module.h"
