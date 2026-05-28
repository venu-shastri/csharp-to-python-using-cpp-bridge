# Complete C++ Bridge Layer

# Folder Structure

```text
cppbridge/
│
├── bridge/
│   ├── bridge.h
│   └── bridge.cpp
│
├── pythonmodule/
│   ├── py_module.cpp
│   └── py_methods.cpp
│
└── CMakeLists.txt
```

---

# bridge/bridge.h

```cpp
#pragma once

class EdaBridge
{
public:

    static int get_address_map_count(
        int interfaceId);

    static int get_address_key_at(
        int interfaceId,
        int index);

    static int get_register_handle_at(
        int interfaceId,
        int index);
};
```

---

# bridge/bridge.cpp

```cpp
#include "bridge.h"

#ifdef _WIN32

#include <Windows.h>

#else

#include <dlfcn.h>

#endif

#include <stdexcept>

static void* libraryHandle = nullptr;

using get_address_map_count_fn =
    int(*)(int);

using get_address_key_at_fn =
    int(*)(int, int);

using get_register_handle_at_fn =
    int(*)(int, int);

static get_address_map_count_fn
    get_address_map_count_native =
        nullptr;

static get_address_key_at_fn
    get_address_key_at_native =
        nullptr;

static get_register_handle_at_fn
    get_register_handle_at_native =
        nullptr;

static void load_library()
{
    if (libraryHandle)
    {
        return;
    }

#ifdef _WIN32

    libraryHandle =
        LoadLibraryA(
            "EdaModel.dll");

    if (!libraryHandle)
    {
        throw std::runtime_error(
            "Failed to load EdaModel.dll");
    }

    get_address_map_count_native =
        reinterpret_cast<
            get_address_map_count_fn>(
                GetProcAddress(
                    (HMODULE)libraryHandle,
                    "get_address_map_count"));

    get_address_key_at_native =
        reinterpret_cast<
            get_address_key_at_fn>(
                GetProcAddress(
                    (HMODULE)libraryHandle,
                    "get_address_key_at"));

    get_register_handle_at_native =
        reinterpret_cast<
            get_register_handle_at_fn>(
                GetProcAddress(
                    (HMODULE)libraryHandle,
                    "get_register_handle_at"));

#else

    libraryHandle =
        dlopen(
            "./libEdaModel.so",
            RTLD_LAZY);

    if (!libraryHandle)
    {
        throw std::runtime_error(
            dlerror());
    }

    get_address_map_count_native =
        reinterpret_cast<
            get_address_map_count_fn>(
                dlsym(
                    libraryHandle,
                    "get_address_map_count"));

    get_address_key_at_native =
        reinterpret_cast<
            get_address_key_at_fn>(
                dlsym(
                    libraryHandle,
                    "get_address_key_at"));

    get_register_handle_at_native =
        reinterpret_cast<
            get_register_handle_at_fn>(
                dlsym(
                    libraryHandle,
                    "get_register_handle_at"));

#endif

    if (!get_address_map_count_native)
    {
        throw std::runtime_error(
            "Failed to load symbol: "
            "get_address_map_count");
    }

    if (!get_address_key_at_native)
    {
        throw std::runtime_error(
            "Failed to load symbol: "
            "get_address_key_at");
    }

    if (!get_register_handle_at_native)
    {
        throw std::runtime_error(
            "Failed to load symbol: "
            "get_register_handle_at");
    }
}

int EdaBridge::get_address_map_count(
    int interfaceId)
{
    load_library();

    return get_address_map_count_native(
        interfaceId);
}

int EdaBridge::get_address_key_at(
    int interfaceId,
    int index)
{
    load_library();

    return get_address_key_at_native(
        interfaceId,
        index);
}

int EdaBridge::get_register_handle_at(
    int interfaceId,
    int index)
{
    load_library();

    return get_register_handle_at_native(
        interfaceId,
        index);
}
```

---

# pythonmodule/py_module.cpp

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

# pythonmodule/py_methods.cpp

```cpp
#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include <exception>

#include "../bridge/bridge.h"

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

---

# CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)

project(eda_native)

set(CMAKE_CXX_STANDARD 17)

find_package(
    Python REQUIRED
    COMPONENTS Interpreter Development)

add_library(
    eda_native
    MODULE

    bridge/bridge.cpp

    pythonmodule/py_module.cpp

    pythonmodule/py_methods.cpp
)

target_include_directories(
    eda_native
    PRIVATE
    ${Python_INCLUDE_DIRS}
)

target_link_libraries(
    eda_native
    PRIVATE
    ${Python_LIBRARIES}
)

if(UNIX)

    target_link_libraries(
        eda_native
        PRIVATE
        dl)

endif()

set_target_properties(
    eda_native
    PROPERTIES
    PREFIX ""
)
```

---

# Build Commands

## Linux

```bash
mkdir build

cd build

cmake ..

cmake --build .
```

Generated:

```text
eda_native.so
```

---

## Windows

```bash
mkdir build

cd build

cmake ..

cmake --build . --config Release
```

Generated:

```text
eda_native.pyd
```
