# py_module.cpp

```cpp
#define PY_SSIZE_T_CLEAN

#include <Python.h>

extern PyMethodDef EdaMethods[];

static struct PyModuleDef
eda_native_module =
{
    PyModuleDef_HEAD_INIT,

    "eda_native",

    "EDA Native Interop Module",

    -1,

    EdaMethods
};

PyMODINIT_FUNC
PyInit_eda_native(void)
{
    return PyModule_Create(
        &eda_native_module);
}
```

---

# py_methods.cpp

```cpp
#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include <exception>

#include "bridge.h"

static PyObject*
py_get_address_map_count(
    PyObject* self,
    PyObject* args)
{
    int interfaceId;

    if (!PyArg_ParseTuple(
            args,
            "i",
            &interfaceId))
    {
        return nullptr;
    }

    try
    {
        int result =
            EdaBridge::get_address_map_count(
                interfaceId);

        return PyLong_FromLong(
            result);
    }
    catch (const std::exception& ex)
    {
        PyErr_SetString(
            PyExc_RuntimeError,
            ex.what());

        return nullptr;
    }
}

static PyObject*
py_get_address_key_at(
    PyObject* self,
    PyObject* args)
{
    int interfaceId;
    int index;

    if (!PyArg_ParseTuple(
            args,
            "ii",
            &interfaceId,
            &index))
    {
        return nullptr;
    }

    try
    {
        int result =
            EdaBridge::get_address_key_at(
                interfaceId,
                index);

        return PyLong_FromLong(
            result);
    }
    catch (const std::exception& ex)
    {
        PyErr_SetString(
            PyExc_RuntimeError,
            ex.what());

        return nullptr;
    }
}

static PyObject*
py_get_register_handle_at(
    PyObject* self,
    PyObject* args)
{
    int interfaceId;
    int index;

    if (!PyArg_ParseTuple(
            args,
            "ii",
            &interfaceId,
            &index))
    {
        return nullptr;
    }

    try
    {
        int result =
            EdaBridge::get_register_handle_at(
                interfaceId,
                index);

        return PyLong_FromLong(
            result);
    }
    catch (const std::exception& ex)
    {
        PyErr_SetString(
            PyExc_RuntimeError,
            ex.what());

        return nullptr;
    }
}

PyMethodDef EdaMethods[] =
{
    {
        "get_address_map_count",
        py_get_address_map_count,
        METH_VARARGS,
        "Returns dictionary count"
    },

    {
        "get_address_key_at",
        py_get_address_key_at,
        METH_VARARGS,
        "Returns dictionary key at index"
    },

    {
        "get_register_handle_at",
        py_get_register_handle_at,
        METH_VARARGS,
        "Returns register handle at index"
    },

    {
        nullptr,
        nullptr,
        0,
        nullptr
    }
};
```
