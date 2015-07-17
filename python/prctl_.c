// gcc -pthread -O2 -g -pipe -Wall -m64 -fPIC `python-config --includes` -c prctl_.c -o _prctl.o
// gcc -pthread -shared _prctl.o -lpython2.6 -o _prctl.so

#include <Python.h>
#include <sys/prctl.h>


static PyObject* prctl_prctl(PyObject* self, PyObject* args)
{
    long option = 0;
    long arg = 0;
    char *argstr = NULL;
    char name[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    int result;

    if(!PyArg_ParseTuple(args, "l|l", &option, &arg)) {
        if(!PyArg_ParseTuple(args, "ls", &option, &argstr))
            return NULL;
        if(option != PR_SET_NAME) {
            PyErr_SetString(PyExc_TypeError, "an integer is required");
            return NULL;
        }
        PyErr_Clear();
    } else {
        if(option == PR_SET_NAME) {
            PyErr_SetString(PyExc_TypeError, "a string is required");
            return NULL;
        }
    }

    switch (option) {
    case(PR_SET_NAME):
        if (strlen(argstr) > 16) {
            PyErr_SetString(PyExc_ValueError, "name length >16");
            return NULL;
        }
        strncpy(name, argstr, 16);
        break;
    }
    switch (option) {
    case(PR_SET_NAME):
    case(PR_GET_NAME):
        result = prctl(option, name, 0, 0, 0);
        if (result < 0) {
            PyErr_SetFromErrno(PyExc_OSError);
            return NULL;
        }
        if (option == PR_GET_NAME)
            return Py_BuildValue("s", name);
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "Unkown prctl option");
        return NULL;
    }

    Py_RETURN_NONE;  // None is returned by default
}

static int __real_argc = -1;
static char** __real_argv = NULL;
#define _Py_GetArgcArgv Py_GetArgcArgv
static PyObject* prctl_set_proctitle(PyObject* self, PyObject* args)
{
    int argc = 0;
    char **argv;
    int len;
    char *title;

    if (!PyArg_ParseTuple(args, "s", &title))
        return NULL;
    if(__real_argc > 0)  {
        argc = __real_argc;
        argv = __real_argv;
    }
    else {
        _Py_GetArgcArgv(&argc, &argv);
        __real_argc = argc;
        __real_argv = argv;
    }

    if(argc <= 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to locate argc/argv");
        return NULL;
    }
    /* Determine up to where we can write */
    len = (size_t)(argv[argc-1]) + strlen(argv[argc-1]) - (size_t)(argv[0]);
    strncpy(argv[0], title, len);
    if(strlen(title) < len)
        memset(argv[0] + strlen(title), 0, len - strlen(title));
    Py_RETURN_NONE;
}

static PyMethodDef PrctlMethods[] = {
    {"prctl", prctl_prctl, METH_VARARGS, "call prctl"},
    {"set_proctitle", prctl_set_proctitle, METH_VARARGS, "set the process title"},
    {NULL, NULL, 0, NULL}  // sentinel
};

#define namedattribute(x) do {  \
    PyModule_AddIntConstant(_prctl, "PR_GET_" #x, PR_GET_ ##x);  \
    PyModule_AddIntConstant(_prctl, "PR_SET_" #x, PR_SET_ ##x);  \
} while (0)

PyMODINIT_FUNC init_prctl(void)
{
    PyObject* _prctl = Py_InitModule("_prctl", PrctlMethods);
    namedattribute(NAME);
}
