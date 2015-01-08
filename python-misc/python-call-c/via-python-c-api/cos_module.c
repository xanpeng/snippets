#include <Python.h>
#include <math.h>

static PyObject* cos_func(PyObject* self, PyObject* args) {
  double value, answer;
  
  // parse the input, from python float to c double
  if (!PyArg_ParseTuple(args, "d", &value))
    return NULL;
  
  // call cos from libm 
  answer = cos(value);
  
  // construct return value, from c double to python float
  return Py_BuildValue("f", answer); 
}

static PyMethodDef CosMethods[] = {
  // METH_VARARGS, METH_VARARGS|METH_KEYWORDS: calling convention, 
  // METH_VARARGS: 期望python传入tuple
  // METH_KEYWORDS: 期望python传入dict
  {"cos_func", cos_func, METH_VARARGS, "evaluate the cosine"},
  {NULL, NULL, 0, NULL}
};

// module initialization
PyMODINIT_FUNC
initcos_module(void) {
  (void) Py_InitModule("cos_module", CosMethods);
}
