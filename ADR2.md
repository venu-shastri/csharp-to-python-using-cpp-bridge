# ADR-001-python-csharp-interop.md

````markdown id="e5n8a2"
# ADR-001
# Python ↔ C# Interoperability Using NativeAOT + C++ Bridge + Runtime Semantic Reconstruction

---

# Status

Accepted

---

# Context

The system uses UML-generated domain models in both:

- C#
- Python

The generated classes are structurally equivalent.

Example:

C#:

```csharp
public sealed class InterfaceDefinition
{
    public List<RegisterDefinition> Registers
        = new();
}
````

Python:

```python
class InterfaceDefinition:

    def __init__(self):

        self.registers = []
```

The architecture requires:

* implementing extension methods in C#
* exposing them naturally to Python
* preserving object identity
* avoiding graph duplication
* avoiding CLR hosting
* avoiding JSON serialization
* keeping generated models untouched

---

# Example Requirement

C# extension method:

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

Desired Python usage:

```python
interface.get_address_map_to_register()
```

without:

* manual binding
* handles
* ctypes
* serialization
* bridge awareness

---

# Decision

Use:

* NativeAOT
* C ABI exports
* C++ bridge
* CPython extension module
* runtime semantic reconstruction
* automatic constructor patching
* automatic object identity binding

instead of:

* pythonnet
* CLR embedding
* JSON transport
* reflection bridges

---

# High Level Architecture

```text
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
C# Extension Methods
```

---

# Key Architectural Principles

# 1. Generated Models Must Remain Pure

Generated UML models must remain:

* interop unaware
* generator owned
* language native

Forbidden:

```python
self._native_handle
```

Forbidden:

```python
import ctypes
```

Forbidden:

* runtime infrastructure
* bridge logic
* ABI knowledge

inside generated models.

---

# 2. Runtime Owns Interop

Interop infrastructure must exist entirely outside generated models.

Responsibilities:

* handle allocation
* identity tracking
* extension injection
* semantic reconstruction

belong to runtime layers.

---

# 3. Automatic Constructor Patching

Object binding must be automatic.

Users must NEVER call:

```python
bind_object(...)
```

Instead runtime patches constructors dynamically.

Example:

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

# 4. ABI-Safe Projections

Managed collections cannot cross ABI boundaries directly.

Forbidden exports:

```csharp
Dictionary<int, RegisterDefinition>
List<T>
IEnumerable<T>
```

because:

* CLR layouts are runtime-specific
* GC ownership exists
* generics are ABI unstable

Instead collections are projected into traversal APIs.

---

# Example Projection

Instead of exporting:

```csharp
Dictionary<int, RegisterDefinition>
```

export:

```text
get_count()

get_key_at(index)

get_value_handle_at(index)
```

This is called:

```text
ABI-safe semantic projection
```

---

# 5. Runtime Semantic Reconstruction

Python reconstructs native semantics from projections.

Example:

```python
result = {}

for i in range(count):

    key = ...

    value = ...

    result[key] = value
```

Users see:

* natural dictionaries
* natural iterables
* natural extension methods

without seeing:

* traversal APIs
* handles
* projection semantics

---

# 6. Extension Method Injection

C# extension methods are dynamically injected into Python models.

Example:

```python
InterfaceDefinition \
    .get_address_map_to_register = (
        get_address_map_to_register
)
```

Result:

```python
interface.get_address_map_to_register()
```

behaves naturally.

---

# 7. No JSON Serialization

JSON transport was rejected.

Reasons:

* identity loss
* synchronization loss
* graph duplication
* excessive allocations
* poor scalability
* detached runtime behavior

---

# 8. No CLR Hosting

Rejected:

* pythonnet
* embedded CLR
* reflection invocation

Reasons:

* runtime coupling
* deployment complexity
* scalability concerns
* versioning risks

---

# 9. NativeAOT + C ABI

.NET exports use:

```xml
<PublishAot>true</PublishAot>

<NativeLib>Shared</NativeLib>
```

Exports use:

```csharp
[UnmanagedCallersOnly]
```

This exposes stable:

* C ABI exports
* native symbols
* runtime-independent binaries

---

# Project Structure

# .NET

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

# C++ Bridge

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

# Python Runtime

```text
python/
│
├── generated/
│
├── runtime/
│   ├── interop_registry.py
│   ├── runtime_patch.py
│   ├── extension_methods.py
│   └── native_loader.py
│
├── bootstrap/
│   └── initialize_runtime.py
│
├── eda_native.pyd
└── EdaModel.dll
```

---

# Runtime Flow

```text
Python Object Creation
        ↓
Constructor Patch
        ↓
Automatic Handle Allocation
        ↓
Runtime Identity Registry
        ↓
Extension Method Invocation
        ↓
CPython Extension Module
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

# Complete Example

# C# Extension Method

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
{
    return definition.Registers.ToDictionary(
        register => register.Offset,
        register => register);
}
```

---

# ABI Projection Exports

```csharp
get_address_map_count()

get_address_key_at(index)

get_register_handle_at(index)
```

---

# Python Semantic Reconstruction

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

# Final User Experience

Users only see:

```python
interface.get_address_map_to_register()
```

Users never see:

* handles
* ABI concepts
* projection APIs
* bridge infrastructure
* runtime registration
* semantic reconstruction

---

# Consequences

# Positive

* clean Python APIs
* scalable interoperability
* ABI-safe architecture
* preserved object identity
* runtime synchronization
* generator independence
* no graph duplication
* no CLR dependency in Python

---

# Negative

* runtime infrastructure complexity
* projection generation overhead
* iterator lifetime management complexity
* custom binding runtime maintenance

---

# Alternatives Considered

# JSON Transport

Rejected because:

* detached objects
* identity loss
* synchronization issues

---

# pythonnet

Rejected because:

* CLR hosting
* runtime coupling
* deployment complexity

---

# Reflection Bridges

Rejected because:

* performance concerns
* ABI instability
* runtime fragility

---

# Future Enhancements

Planned support:

* iterator projections
* async projections
* lazy graph traversal
* automatic iterator disposal
* graph streaming
* cached projections
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
* scalable ABI-safe interoperability

```
```
