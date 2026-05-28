# Step-by-Step Implementation Guide

## Python ↔ C# Interoperability Using NativeAOT + C++ Bridge + CPython Extension

---

# Goal

Implement the following Python API:

```python
interface.get_address_map_to_register()
```

where actual implementation logic exists in C#:

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

using:

* NativeAOT
* C ABI exports
* C++ bridge
* CPython extension module
* runtime semantic reconstruction

without:

* JSON serialization
* pythonnet
* CLR hosting
* modifying generated Python models

---

# Final Architecture

```text
Generated Python Models
        ↓
Runtime Patch Layer
        ↓
CPython Extension Module
        ↓
C++ Bridge
        ↓
NativeAOT Shared Library
        ↓
C# Extension Methods
```

---

# Step 1 — Create Solution Structure

Create folders:

```text
EdaInteropSolution/
│
├── dotnet/
│   └── EdaModel/
│
├── cppbridge/
│
├── python/
│   ├── generated/
│   ├── runtime/
│   └── bootstrap/
│
└── tests/
```

---

# Step 2 — Create .NET NativeAOT Project

# Create Project

```bash
dotnet new classlib -n EdaModel
```

---

# Update EdaModel.csproj

```xml
<Project Sdk="Microsoft.NET.Sdk">

  <PropertyGroup>

    <TargetFramework>net8.0</TargetFramework>

    <PublishAot>true</PublishAot>

    <NativeLib>Shared</NativeLib>

    <SelfContained>true</SelfContained>

    <InvariantGlobalization>true</InvariantGlobalization>

  </PropertyGroup>

</Project>
```

---

# Step 3 — Create Domain Models

# InterfaceDefinition.cs

```csharp
namespace EdaModel;

public sealed class InterfaceDefinition
{
    public List<RegisterDefinition> Registers { get; }
        = new();
}
```

---

# RegisterDefinition.cs

```csharp
namespace EdaModel;

public sealed class RegisterDefinition
{
    public string Name { get; set; } = "";

    public int Offset { get; set; }
}
```

---

# Step 4 — Add Extension Method

# InterfaceDefinitionExtensions.cs

```csharp
namespace EdaModel;

public static class InterfaceDefinitionExtensions
{
    public static Dictionary<int, RegisterDefinition>
        GetAddressMapToRegister(
            this InterfaceDefinition definition)
    {
        return definition.Registers.ToDictionary(
            register => register.Offset,
            register => register);
    }
}
```

---

# Step 5 — Create Runtime Object Registry

We need stable handles.

Create:

```csharp
Runtime/ObjectRegistry.cs
```

---

# ObjectRegistry.cs

```csharp
namespace EdaModel;

public static class ObjectRegistry
{
    private static readonly Dictionary<
        int,
        InterfaceDefinition> Interfaces = new();

    private static readonly Dictionary<
        int,
        RegisterDefinition> Registers = new();

    private static int _nextId = 1;

    public static int Register(
        InterfaceDefinition definition)
    {
        var id = _nextId++;

        Interfaces[id] = definition;

        return id;
    }

    public static int Register(
        RegisterDefinition definition)
    {
        var id = _nextId++;

        Registers[id] = definition;

        return id;
    }

    public static InterfaceDefinition
        GetInterface(int id)
    {
        return Interfaces[id];
    }

    public static RegisterDefinition
        GetRegister(int id)
    {
        return Registers[id];
    }
}
```

---

# Step 6 — Create Native ABI Projection Exports

Create:

```text
Exports/Exported.cs
```

---

# Exported.cs

```csharp
using System.Runtime.InteropServices;

namespace EdaModel;

public static class Exported
{
    [UnmanagedCallersOnly(
        EntryPoint = "get_address_map_count")]
    public static int GetAddressMapCount(
        int interfaceId)
    {
        return ObjectRegistry
            .GetInterface(interfaceId)
            .GetAddressMapToRegister()
            .Count;
    }

    [UnmanagedCallersOnly(
        EntryPoint = "get_address_key_at")]
    public static int GetAddressKeyAt(
        int interfaceId,
        int index)
    {
        return ObjectRegistry
            .GetInterface(interfaceId)
            .GetAddressMapToRegister()
            .Keys
            .ElementAt(index);
    }

    [UnmanagedCallersOnly(
        EntryPoint = "get_register_handle_at")]
    public static int GetRegisterHandleAt(
        int interfaceId,
        int index)
    {
        var register =
            ObjectRegistry
                .GetInterface(interfaceId)
                .GetAddressMapToRegister()
                .Values
                .ElementAt(index);

        return ObjectRegistry
            .Register(register);
    }
}
```

---

# Important Concept

We are NOT exporting:

```csharp
Dictionary<int, RegisterDefinition>
```

Instead we export:

* count
* key lookup
* value lookup

This is called:

```text
ABI-safe semantic projection
```

---

# Step 7 — Publish Native Library

Windows:

```bash
dotnet publish \
-c Release \
-r win-x64
```

Linux:

```bash
dotnet publish \
-c Release \
-r linux-x64
```

Generated:

* `EdaModel.dll`
* or `libEdaModel.so`

---

# Step 8 — Create C++ Bridge

# bridge.h

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

# bridge.cpp

```cpp
#include "bridge.h"

#ifdef _WIN32

#include <Windows.h>

#else

#include <dlfcn.h>

#endif

static void* libraryHandle = nullptr;

using get_address_map_count_fn =
    int(*)(int);

using get_address_key_at_fn =
    int(*)(int, int);

using get_register_handle_at_fn =
    int(*)(int, int);

static get_address_map_count_fn
    get_address_map_count_native;

static get_address_key_at_fn
    get_address_key_at_native;

static get_register_handle_at_fn
    get_register_handle_at_native;

static void load_library()
{
    if (libraryHandle)
        return;

#ifdef _WIN32

    libraryHandle =
        LoadLibraryA("EdaModel.dll");

    get_address_map_count_native =
        (get_address_map_count_fn)
        GetProcAddress(
            (HMODULE)libraryHandle,
            "get_address_map_count");

#endif
}
```

---

# Implement Functions

```cpp
int EdaBridge::get_address_map_count(
    int interfaceId)
{
    load_library();

    return get_address_map_count_native(
        interfaceId);
}
```

Repeat for:

* `get_address_key_at`
* `get_register_handle_at`

---

# Step 9 — Create CPython Extension Module

# py_module.cpp

```cpp
#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "bridge.h"
```

---

# Export Python Function

```cpp
static PyObject*
get_address_map_count(
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

    auto result =
        EdaBridge::get_address_map_count(
            interfaceId);

    return PyLong_FromLong(result);
}
```

---

# Register Methods

```cpp
static PyMethodDef Methods[] =
{
    {
        "get_address_map_count",
        get_address_map_count,
        METH_VARARGS,
        ""
    },

    {nullptr, nullptr, 0, nullptr}
};
```

---

# Create Python Module

```cpp
static PyModuleDef module =
{
    PyModuleDef_HEAD_INIT,
    "eda_native",
    nullptr,
    -1,
    Methods
};

PyMODINIT_FUNC
PyInit_eda_native(void)
{
    return PyModule_Create(
        &module);
}
```

---

# Step 10 — Create CMake File

# CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)

project(eda_native)

find_package(
    Python REQUIRED
    COMPONENTS Interpreter Development)

add_library(
    eda_native
    MODULE
    py_module.cpp
    bridge.cpp)

include_directories(
    ${Python_INCLUDE_DIRS})

target_link_libraries(
    eda_native
    ${Python_LIBRARIES})

if(UNIX)
    target_link_libraries(
        eda_native
        dl)
endif()
```

---

# Step 11 — Build CPython Extension

```bash
mkdir build

cd build

cmake ..

cmake --build .
```

Generated:

* `eda_native.pyd`
* or `eda_native.so`

---

# Step 12 — Create Generated Python Models

# generated/interface_definition.py

```python
class InterfaceDefinition:

    def __init__(self):

        self.registers = []
```

---

# generated/register_definition.py

```python
class RegisterDefinition:

    def __init__(self):

        self.name = ""

        self.offset = 0
```

---

# Important Rule

Generated models must remain:

* pure
* interop unaware
* UML generated

No native handles allowed.

---

# Step 13 — Create Runtime Registry

# runtime/interop_registry.py

```python
import weakref

_object_to_handle = (
    weakref.WeakKeyDictionary()
)

_handle_to_object = {}

def bind_object(
    python_object,
    handle):

    _object_to_handle[
        python_object
    ] = handle

    _handle_to_object[
        handle
    ] = python_object

def get_handle(obj):

    return _object_to_handle[obj]

def get_python_object(handle):

    return _handle_to_object[handle]
```

---

# Step 14 — Create Runtime Extension Method

# runtime/extension_methods.py

```python
import eda_native

from generated.interface_definition \
    import InterfaceDefinition

from runtime.interop_registry \
    import get_handle

from runtime.interop_registry \
    import get_python_object
```

---

# Create Semantic Reconstruction

```python
def get_address_map_to_register(
    self):

    handle = get_handle(self)

    count = (
        eda_native
            .get_address_map_count(
                handle)
    )

    result = {}

    for i in range(count):

        key = (
            eda_native
                .get_address_key_at(
                    handle,
                    i)
        )

        register_handle = (
            eda_native
                .get_register_handle_at(
                    handle,
                    i)
        )

        register = (
            get_python_object(
                register_handle)
        )

        result[key] = register

    return result
```

---

# Inject Method Into Generated Class

```python
InterfaceDefinition \
    .get_address_map_to_register = (
        get_address_map_to_register
)
```

---

# Step 15 — Create Bootstrap Initialization

# bootstrap/initialize_runtime.py

```python
import runtime.extension_methods
```

---

# Step 16 — Final User Experience

# user_script.py

```python
from bootstrap.initialize_runtime import *

from generated.interface_definition \
    import InterfaceDefinition

interface = InterfaceDefinition()

result = (
    interface
        .get_address_map_to_register()
)

print(result)
```

---

# Final Runtime Flow

```text
Python Object
        ↓
Handle Lookup
        ↓
CPython Extension
        ↓
C++ Bridge
        ↓
NativeAOT Export
        ↓
C# Extension Method
        ↓
Projection APIs
        ↓
Python Semantic Reconstruction
```

---

# Important Design Principles

# Do NOT Export Managed Collections

Forbidden:

```csharp
Dictionary<int, RegisterDefinition>
```

Allowed:

```text
count()
item_at()
key_at()
value_at()
```

---

# Do NOT Use JSON

Reason:

* identity loss
* object duplication
* synchronization problems
* scalability issues

---

# Do NOT Embed Native Handles

Generated models must remain:

* portable
* generator-owned
* runtime-independent

---

# Final Result

The user gets natural Python APIs:

```python
interface.get_address_map_to_register()
```

while internally:

* managed objects stay in CLR
* Python objects stay in CPython
* ABI boundary remains safe
* object identity is preserved
* extension methods remain implemented in C#

```
```

