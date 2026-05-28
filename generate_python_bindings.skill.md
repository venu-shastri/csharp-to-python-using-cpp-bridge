# generate_python_bindings.skill.md

````markdown
---
name: GeneratePythonBindings
version: 2.0
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

The generated system must provide natural Python APIs while preserving:

- object identity
- runtime synchronization
- scalability
- ABI safety

---

# High Level Architecture

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
C# Domain Models + Extension Methods

---

# Responsibilities

This skill generates:

- ABI projection exports
- C++ bridge code
- CPython extension module
- runtime patch modules
- semantic reconstruction adapters
- iterable adapters
- dictionary adapters
- extension method injectors
- runtime registries

---

# Non Responsibilities

This skill must NOT:

- modify generated UML model classes
- embed handles inside generated classes
- host CLR inside Python
- serialize managed graphs through JSON
- duplicate managed object graphs

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
│   ├── extension_methods.py
│   ├── runtime_patch.py
│   └── native_loader.py
│
├── semantic_views/
│   ├── list_view.py
│   ├── dictionary_view.py
│   └── iterable_view.py
│
└── bootstrap/
    └── initialize_runtime.py
````

---

# Generated .NET Folder Structure

```text
dotnet/
│
└── EdaModel/
    ├── Models/
    ├── Extensions/
    ├── Exports/
    └── Runtime/
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

# Core Rules

# Rule 1 — Generated Models Must Remain Untouched

Generated Python classes must remain:

* pure UML-generated
* language-native
* interop unaware

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
native bridge logic
```

---

# Rule 2 — Interop Uses External Registry

Object identity must be tracked using:

```python
WeakKeyDictionary
```

Required mapping:

```text
Python Object ↔ Handle ↔ Managed Object
```

---

# Rule 3 — Export ABI-Safe Types Only

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
managed collections
managed objects
generic types
IEnumerable<T>
Dictionary<TKey, TValue>
List<T>
```

---

# Rule 4 — Collections Must Use Projection Semantics

Managed collections must be decomposed into traversal semantics.

---

# List<T> Projection

C#:

```csharp
List<RegisterDefinition>
```

Generate:

```text
count()
item_at(index)
```

Generate Python semantic wrapper:

```python
class ListView:
    def __len__(self):
        ...

    def __getitem__(self, index):
        ...

    def __iter__(self):
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

Generate Python semantic wrapper:

```python
class DictionaryView:
    def items(self):
        ...

    def keys(self):
        ...

    def values(self):
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
iterator_destroy()
```

OR indexed traversal projection.

---

# Rule 5 — Runtime Semantic Reconstruction

Generated runtime layer reconstructs:

* list semantics
* iterable semantics
* dictionary semantics

from projection APIs.

Users must never see raw projection functions.

---

# Rule 6 — Extension Method Injection

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

Generated code:

```python
InterfaceDefinition \
    .get_address_map_to_register = (
        get_address_map_to_register
)
```

---

# Rule 7 — No JSON Transport

Forbidden:

```csharp
string ExportAsJson()
```

Reason:

* identity loss
* synchronization loss
* excessive allocations
* object duplication

---

# Rule 8 — No CLR Hosting

Forbidden:

* pythonnet
* embedded CLR
* reflection-based runtime invocation

Reason:

* runtime coupling
* scalability issues
* deployment complexity

---

# Rule 9 — Use NativeAOT

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

# Rule 10 — Use Lazy Traversal

Avoid:

* materializing large graphs
* eager collection reconstruction

Prefer:

* lazy iterators
* semantic views
* projection traversal

---

# Example Mapping

# C#

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

---

# Generated ABI Projection

```text
get_address_map_count()

get_address_key_at(index)

get_register_handle_at(index)
```

---

# Generated Python Semantic Wrapper

```python
def get_address_map_to_register(
    self):

    handle = get_handle(self)

    count = native.get_address_map_count(
        handle)

    result = {}

    for i in range(count):

        key = native.get_address_key_at(
            handle,
            i)

        value_handle = (
            native.get_register_handle_at(
                handle,
                i)
        )

        value = get_python_object(
            value_handle)

        result[key] = value

    return result
```

---

# Performance Guidelines

Prefer:

* handles
* projections
* lazy traversal
* iterator semantics

Avoid:

* reflection
* serialization
* graph duplication
* runtime object copying

---

# Memory Ownership Rules

Managed objects:

* owned by CLR

Python objects:

* owned by CPython

Interop layer:

* never transfers ownership
* only transfers handles

---

# Runtime Bootstrap Rules

Generated startup must initialize:

```python
import runtime.runtime_patch
import runtime.extension_methods
```

before user code executes.

---

# Future Enhancements

Planned future generation support:

* async iterators
* graph streaming
* incremental synchronization
* cached projections
* lazy dependency traversal
* proxy collection views
* automatic iterator lifetime management

---

# Final Design Principle

The interoperability layer is:

```text
a generated semantic binding architecture
```

NOT:

* a serialization framework
* a reflection bridge
* a scripting adapter

The goal is:

* natural Python semantics
* preserved managed behavior
* scalable interoperability
* ABI-safe runtime boundaries

```
```

