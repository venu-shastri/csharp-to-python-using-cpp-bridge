# generate_python_bindings.skill.md

````markdown
---
name: GeneratePythonBindings
version: 3.0
language: cpp-csharp-python
type: interoperability-generation-skill
---

# Goal

Generate Python bindings for UML-generated C# domain models and extension methods using:

- NativeAOT
- C ABI exports
- C++ bridge
- CPython extension module
- runtime semantic reconstruction
- automatic object binding

The generated system must provide:

- natural Python APIs
- preserved object identity
- runtime synchronization
- ABI-safe interoperability
- hidden interop infrastructure

without:
- JSON serialization
- CLR hosting
- pythonnet
- user-visible handles
- user-visible binding APIs

---

# High Level Architecture

Generated Python Models
        ↓
Runtime Patch Layer
        ↓
Automatic Handle Binding
        ↓
Extension Method Injection
        ↓
CPython Extension Module
        ↓
C++ Bridge
        ↓
NativeAOT Shared Library
        ↓
C# Domain Models + Extension Methods

---

# Responsibilities

This skill generates:

- ABI projection exports
- C++ bridge code
- CPython extension module
- runtime semantic wrappers
- iterable projections
- dictionary projections
- automatic constructor patching
- extension method injectors
- object identity registries
- semantic reconstruction layers

---

# Non Responsibilities

This skill must NOT:

- modify generated UML classes
- embed native handles into models
- expose bind APIs to users
- host CLR inside Python
- serialize managed graphs
- duplicate managed objects

---

# Generated Folder Structure

```text
python/
│
├── generated/
│   ├── interface_definition.py
│   ├── register_definition.py
│   └── ...
│
├── runtime/
│   ├── interop_registry.py
│   ├── runtime_patch.py
│   ├── extension_methods.py
│   ├── native_loader.py
│   └── object_factory.py
│
├── bootstrap/
│   └── initialize_runtime.py
│
├── eda_native.pyd
└── EdaModel.dll
````

---

# Generated .NET Structure

```text
dotnet/
│
└── EdaModel/
    ├── Models/
    ├── Extensions/
    ├── Runtime/
    └── Exports/
```

---

# Generated C++ Structure

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

# Rule 1 — Generated Models Must Remain Pure

Generated UML Python models must remain:

* interop unaware
* generator owned
* pure language models

Forbidden:

```python
self._native_handle
```

Forbidden:

```python
import ctypes
```

Forbidden:

```python
interop code inside generated models
```

---

# Rule 2 — Runtime Owns Identity Management

Object identity must be managed externally using:

```python
WeakKeyDictionary
```

Required mapping:

```text
Python Object ↔ Native Handle ↔ Managed Object
```

---

# Rule 3 — Automatic Constructor Patching

The runtime must automatically patch constructors of generated models.

Users must NEVER call:

```python
bind_object(...)
```

Allowed user experience:

```python
interface = InterfaceDefinition()
```

Runtime automatically:

* allocates handles
* registers object identity
* binds runtime mappings

---

# Required Runtime Patch Pattern

```python
def patch_constructor(cls):

    original_init = cls.__init__

    def new_init(
        self,
        *args,
        **kwargs):

        original_init(
            self,
            *args,
            **kwargs)

        handle = allocate_handle()

        bind_object(
            self,
            handle)

    cls.__init__ = new_init
```

---

# Rule 4 — Export ABI-Safe Types Only

Allowed exports:

```text
int
long
double
char*
opaque handles
```

Forbidden exports:

```text
Dictionary<TKey, TValue>
List<T>
IEnumerable<T>
managed objects
generic collections
```

---

# Rule 5 — Collection Projection Semantics

Managed collections must be projected into traversal APIs.

---

# List Projection

C#:

```csharp
List<RegisterDefinition>
```

Generate:

```text
count()
item_at(index)
```

Reconstruct Python semantics using:

```python
class ListView:
    ...
```

---

# Dictionary Projection

C#:

```csharp
Dictionary<int, RegisterDefinition>
```

Generate:

```text
count()
key_at(index)
value_at(index)
```

Reconstruct Python semantics using:

```python
class DictionaryView:
    ...
```

---

# IEnumerable Projection

C#:

```csharp
IEnumerable<Component>
```

Generate:

```text
iterator_create()
iterator_next()
iterator_current()
```

OR indexed traversal projections.

---

# Rule 6 — Runtime Semantic Reconstruction

The runtime layer reconstructs:

* dictionary semantics
* iterable semantics
* list semantics

from ABI-safe projections.

Users must never see:

* projection functions
* handles
* traversal primitives

---

# Rule 7 — Extension Method Injection

C# extension methods:

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

must become:

```python
interface.get_address_map_to_register()
```

through runtime injection.

Required pattern:

```python
InterfaceDefinition \
    .get_address_map_to_register = (
        get_address_map_to_register
)
```

---

# Rule 8 — No JSON Transport

Forbidden:

```csharp
string ExportAsJson()
```

Reason:

* identity loss
* synchronization loss
* graph duplication
* allocation overhead

---

# Rule 9 — No CLR Hosting

Forbidden:

* pythonnet
* embedded CLR
* reflection-based invocation

Reason:

* runtime coupling
* deployment complexity
* scalability limitations

---

# Rule 10 — Use NativeAOT

All .NET exports must use:

```xml
<PublishAot>true</PublishAot>
<NativeLib>Shared</NativeLib>
```

Exports must use:

```csharp
[UnmanagedCallersOnly]
```

---

# Rule 11 — Use Lazy Traversal

Avoid:

* eager graph materialization
* deep object duplication
* full graph serialization

Prefer:

* projection traversal
* iterators
* semantic wrappers

---

# Example Mapping

# C#

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

---

# ABI Projection

```text
get_address_map_count()

get_address_key_at(index)

get_register_handle_at(index)
```

---

# Runtime Semantic Reconstruction

```python
def get_address_map_to_register(
    self):

    interface_handle = (
        get_handle(self)
    )

    count = (
        eda_native
            .get_address_map_count(
                interface_handle)
    )

    result = {}

    for i in range(count):

        key = (
            eda_native
                .get_address_key_at(
                    interface_handle,
                    i)
        )

        register_handle = (
            eda_native
                .get_register_handle_at(
                    interface_handle,
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

# Runtime Bootstrap Rules

Generated startup must initialize:

```python
import runtime.runtime_patch

import runtime.extension_methods
```

before user code executes.

---

# Final User Experience

User only writes:

```python
interface = InterfaceDefinition()

result = (
    interface
        .get_address_map_to_register()
)
```

Users must never see:

* handles
* ABI concepts
* native interop
* runtime registration
* semantic reconstruction infrastructure

---

# Performance Guidelines

Prefer:

* handles
* lazy traversal
* iterator semantics
* semantic projections

Avoid:

* JSON serialization
* reflection
* graph duplication
* runtime copying

---

# Memory Ownership Rules

Managed objects:

* owned by CLR

Python objects:

* owned by CPython

Interop layer:

* never transfers ownership
* only transfers opaque handles

---

# Future Enhancements

Planned future support:

* async iterators
* graph streaming
* cached projections
* incremental synchronization
* lazy graph traversal
* automatic iterator disposal
* proxy collection views

---

# Final Design Principle

The interoperability layer is:

```text
a generated semantic binding architecture
```

NOT:

* a serialization framework
* a reflection bridge
* a CLR embedding system

Goal:

* natural Python semantics
* preserved managed behavior
* scalable interoperability
* ABI-safe runtime boundaries

```
```
