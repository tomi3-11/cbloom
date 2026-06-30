#define PY_SIZE_T_CLEAN
#include <Python.h>
#include "bloom.h"

/* Python object wrapping BloomFilter */

typedef struct {
    PyObject_HEAD // Mandatory macro -> adds refcount + type pointer
    BloomFilter *bf; // the C struct
} PyBloomObject;

static PyObject *PyBloom_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyBloomObject *self = (PyBloomObject *)type->tp_alloc(type, 0);
    if (self)
        self->bf = NULL;
    return (PyObject *)self;
}

static int PyBloom_init(PyBloomObject *self, PyObject *args, PyObject *kwds)
{
    uint64_t capacity;
    double error_rate;

    // "Kd" means: unsigned long long, double
    if (!PyArg_ParseTuple(args, "Kd", &capacity, &error_rate))
        return -1;

    self->bf = bloom_create(capacity, error_rate);
    if (!self->bf) {
        PyErr_SetString(PyExc_ValueError, "Invalid capacity or error_rate");
        return -1;
    }
    return 0;
}

static void PyBloom_dealloc(PyBloomObject *self)
{
    bloom_free(self->bf);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *PyBloom_add(PyBloomObject *self, PyObject *args)
{
    const char *key;
    Py_ssize_t len;

    // "s#" means: string pointer + its length
    if (!PyArg_ParseTuple(args, "s#", &key, &len))
        return NULL;

    bloom_add(self->bf, key, (size_t)len);
    Py_RETURN_NONE;
}

static PyObject *PyBloom_contains(PyBloomObject *self, PyObject *args)
{
    const char *key;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, "s#", &key, &len))
        return NULL;

    int found = bloom_contains(self->bf, key, (size_t)len);
    return PyBool_FromLong(found); // returns True or False
}

static PyObject *PyBloom_item_count(PyBloomObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromUnsignedLongLong(self->bf->item_count);
}

static PyObject *PyBloom_bit_count(PyBloomObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromUnsignedLongLong(self->bf->bit_count);
}

static PyObject *PyBloom_hash_count(PyBloomObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyLong_FromUnsignedLongLong(self->bf->hash_count);
}

// Method table
static PyMethodDef PyBloom_methods[] = {
    {"add", (PyCFunction)PyBloom_add, METH_VARARGS, "Add a keyy to the filter."},
    {"contains", (PyCFunction)PyBloom_contains, METH_VARARGS, "Return True if key is probably present, False if definitely absent."},
    {"item_count", (PyCFunction)PyBloom_item_count, METH_NOARGS, "Number of items inserted."},
    {"bit_count", (PyCFunction)PyBloom_bit_count, METH_NOARGS, "Total bits in the filter (m)."},
    {"hash_count", (PyCFunction)PyBloom_hash_count, METH_NOARGS, "Number of hash functions (k)."},
    {NULL, NULL, 0, NULL} // Sentinel -> marks end of table
};

// type definition
static PyTypeObject PyBloomType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_cbloom.BloomFilter",
    .tp_basicsize = sizeof(PyBloomObject),
    .tp_dealloc = (destructor)PyBloom_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Bloom filter backed by C.",
    .tp_methods = PyBloom_methods,
    .tp_init = (initproc)PyBloom_init,
    .tp_new = PyBloom_new,
};

// Module Definition
static PyModuleDef cbloom_module = {
    PyModuleDef_HEAD_INIT,
    "_cbloom", // Module name -> underscore means "internal"
    NULL,       // Module docstring
    -1, // -1 = module keeps state in global variables
    NULL
};

PyMODINIT_FUNC PyInit__cbloom(void)
{
    // Finalise the type -> fills in slots we left NULL
    if (PyType_Ready(&PyBloomType) < 0)
        return NULL;

    PyObject *m = PyModule_Create(&cbloom_module);
    if (!m) return NULL;

    // Add BloomFilter class to the module
    Py_INCREF(&PyBloomType);
    if (PyModule_AddObject(m, "BloomFilter", (PyObject*)&PyBloomType) < 0){
        Py_DECREF(&PyBloomType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
