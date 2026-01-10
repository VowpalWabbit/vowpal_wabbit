#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject* hello(PyObject* self, PyObject* args) {
    return PyUnicode_FromString("Hello from simple extension!");
}

static PyMethodDef module_methods[] = {
    {"hello", hello, METH_NOARGS, "Return a greeting"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef simple_module = {
    PyModuleDef_HEAD_INIT,
    "simple_module",
    "A simple test module",
    -1,
    module_methods
};

PyMODINIT_FUNC PyInit_simple_module(void) {
    return PyModule_Create(&simple_module);
}
