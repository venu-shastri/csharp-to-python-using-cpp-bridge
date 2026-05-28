# generate_python_bindings.skill.md

````markdown
---
name: GeneratePythonBindings
version: 2.0
language: cpp-csharp-python
---

# Goal

Generate Python bindings for C# domain models and extension methods.

# Architecture

Generated Python classes remain untouched.

Interop behavior is injected dynamically using:
- runtime patching
- external handle registry
- CPython extension module

# Runtime Architecture

Python Generated Models
        ↓
Runtime Patch Layer
        ↓
CPython Extension Module
        ↓
C++ Bridge
        ↓
NativeAOT Shared Library
        ↓
C# Domain Models

# Rules

## Python Model Rules

Generated Python classes:
- must remain untouched
- must remain pure UML-generated models
- must not contain native handles
- must not contain bridge logic

## Runtime Patch Rules

Generate:
- runtime patch module
- extension method injection
- constructor patching
- handle registry

## Handle Rules

Use:
- WeakKeyDictionary
- external handle registry

Do NOT:
- embed handles in generated models

## Extension Method Rules

C# extension methods:

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
````

must become:

```python
interface.get_address_map_to_register()
```

through runtime method injection.

## Native ABI Rules

Allowed types:

* int
* long
* double
* char*
* object handles

Do NOT expose:

* managed Dictionary
* managed List
* managed objects

## Performance Rules

Avoid:

* JSON serialization
* reflection
* CLR hosting inside Python

Prefer:

* handle-based traversal
* indexed access APIs
* lazy access

## C++ Bridge Rules

Use:

* function pointer loading
* LoadLibrary/GetProcAddress
* dlopen/dlsym

## Memory Rules

Managed objects stay inside:

* CLR heap

Python objects stay inside:

* CPython heap

Only handles cross ABI boundaries.

# Example Mapping

C#:

```csharp
GetAddressMapToRegister()
```

Python:

```python
interface.get_address_map_to_register()
```

# Constraints

* no pythonnet
* no reflection-based interop
* no duplicated object synchronization
* no embedded handles in generated models

```
```
