# Architecture Decision Record

## ADR-001 — Python ↔ C# Interoperability Architecture for UML Generated EDA Models

### Status

Accepted

---

# Context

The system consists of:

* UML-defined domain models
* Generated C# classes
* Generated Python classes
* Shared conceptual object model across languages

The EDA/VLSI domain model includes:

* `System`
* `Component`
* `InterfaceDefinition`
* `RegisterDefinition`
* extension methods
* graph traversal APIs
* dependency analysis APIs

The project requires:

* invoking C# extension methods from Python
* preserving object identity across runtimes
* supporting large object graphs
* avoiding duplicated synchronization logic
* supporting future scalability
* preserving natural Python API semantics

Example desired Python usage:

```python
interface.get_address_map_to_register()
```

while implementation logic exists in C# extension methods:

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

---

# Problem Statement

Direct interoperability between:

* CPython runtime
* .NET CLR runtime

is not possible for:

* managed collections
* managed generics
* managed object graphs
* CLR object references

The following types cannot cross native ABI directly:

```csharp
Dictionary<int, RegisterDefinition>

List<RegisterDefinition>

IEnumerable<Component>

InterfaceDefinition
```

because native ABI supports only:

* primitive values
* pointers
* arrays
* handles

Additionally:

* generated Python classes cannot be modified
* generated Python classes must remain pure UML-generated models
* embedding native handles inside generated classes is not allowed

---

# Decision

Adopt a:

```text
Handle + Projection + Semantic Reconstruction Architecture
```

using:

* .NET NativeAOT
* C ABI exports
* C++ bridge layer
* CPython extension module
* runtime method injection
* external object registry
* generated projection adapters
* generated semantic wrappers

instead of:

* JSON-based transport
* pythonnet
* CLR hosting inside Python
* duplicated object synchronization

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
C# Domain Models + Extension Methods
```

---

# High Level Project Structure

```text
EdaInteropSolution/
│
├── uml/
│   ├── model.uml
│   └── generation-config/
│
├── generators/
│   ├── csharp-generator/
│   ├── python-generator/
│   └── interop-generator/
│
├── dotnet/
│   └── EdaModel/
│       ├── Models/
│       ├── Extensions/
│       ├── Exports/
│       └── Runtime/
│
├── cppbridge/
│   ├── bridge/
│   ├── pythonmodule/
│   └── CMakeLists.txt
│
├── python/
│   ├── generated/
│   ├── runtime/
│   ├── semantic_views/
│   └── bootstrap/
│
├── skill/
│   └── generate_python_bindings.skill.md
│
└── tests/
    ├── csharp/
    ├── cpp/
    └── python/
```

---

# Detailed Runtime Structure

# UML Layer

Responsible for:

* source-of-truth model
* domain definitions
* relationships
* inheritance
* metadata

Outputs:

* generated C# classes
* generated Python classes
* interop metadata

---

# Generator Layer

Responsible for generating:

* C# domain classes
* Python domain classes
* ABI projection exports
* semantic wrappers
* runtime patch modules
* collection adapters
* iterator adapters
* skill metadata

---

# .NET Layer

Responsible for:

* domain behavior
* extension methods
* business logic
* graph analysis
* dependency propagation

Contains:

```text
Models/
Extensions/
Exports/
Runtime/
```

---

# C++ Bridge Layer

Responsible for:

* ABI isolation
* dynamic loading
* platform abstraction
* CPython integration

Contains:

```text
bridge/
pythonmodule/
```

---

# Python Runtime Layer

Responsible for:

* runtime patching
* semantic reconstruction
* object registry
* extension injection
* iterable reconstruction

Contains:

```text
runtime/
semantic_views/
bootstrap/
```

---

# Generated Model Strategy

Generated models remain:

* pure UML generated
* language-native
* interop unaware

Example:

```python
class InterfaceDefinition:

    def __init__(self):

        self.registers = []
```

No:

* handles
* bridge calls
* runtime metadata

are embedded.

---

# Core Design Principles

## 1. Generated Models Remain Untouched

Generated Python classes remain:

* pure UML-generated
* language-native
* unaware of interop

---

## 2. Managed Objects Stay Inside CLR

Managed objects remain owned by:

```text
.NET CLR Heap
```

Python objects remain owned by:

```text
CPython Heap
```

Only stable handles cross runtime boundaries.

---

## 3. Managed Collections Are NOT Exported

The following are never exported directly:

```csharp
Dictionary<TKey, TValue>

List<T>

IEnumerable<T>

HashSet<T>
```

Instead, their semantics are projected through ABI-safe traversal APIs.

---

# Why JSON Was Rejected

Initial design considered:

```csharp
string GetAddressMapToRegisterJson()
```

Reasons for rejection:

| Problem                  | Impact |
| ------------------------ | ------ |
| Loses object identity    | Severe |
| Duplicates object graphs | Severe |
| Expensive serialization  | High   |
| Expensive parsing        | High   |
| No behavior preservation | Severe |
| No synchronization       | Severe |
| Poor scalability         | High   |

---

# Chosen Design

## Semantic Projection

Managed behavior is decomposed into ABI-safe primitives.

Example:

C# extension method:

```csharp
public static Dictionary<int, RegisterDefinition>
GetAddressMapToRegister(
    this InterfaceDefinition definition)
```

becomes:

```text
get_address_map_count()

get_address_key_at(index)

get_register_handle_at(index)
```

Python reconstructs semantic behavior.

---

# Runtime Object Registry

A runtime registry maps:

```text
Python Object ↔ Handle ↔ C# Object
```

using:

```python
WeakKeyDictionary
```

---

# Python Runtime Injection

Generated Python classes are dynamically extended at runtime.

Example:

```python
InterfaceDefinition \
    .get_address_map_to_register = (
        get_address_map_to_register
)
```

This provides natural Python API semantics:

```python
interface.get_address_map_to_register()
```

without modifying generated classes.

---

# Collection Semantics Strategy

Managed collections are projected as semantic adapters.

| C# Type         | Projection          | Python Semantic |
| --------------- | ------------------- | --------------- |
| List<T>         | count + item_at     | sequence        |
| Dictionary<K,V> | key/value traversal | dict-like       |
| IEnumerable<T>  | iterator APIs       | iterable        |
| HashSet<T>      | traversal APIs      | set-like        |

---

# Example: List Projection

C#:

```csharp
List<RegisterDefinition>
```

Projection:

```text
get_count()
get_item_at(index)
```

Python wrapper:

```python
class RegisterListView:

    def __len__(self):
        ...

    def __getitem__(self, index):
        ...

    def __iter__(self):
        ...
```

---

# Detailed Example

# C# Model

## InterfaceDefinition.cs

```csharp
namespace EdaModel;

public sealed class InterfaceDefinition
{
    public List<RegisterDefinition> Registers { get; }
        = new();
}
```

---

## RegisterDefinition.cs

```csharp
namespace EdaModel;

public sealed class RegisterDefinition
{
    public string Name { get; set; } = "";

    public int Offset { get; set; }
}
```

---

## Extension Method

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

# Native ABI Projection

## Exported.cs

```csharp
[UnmanagedCallersOnly(
    EntryPoint = "get_address_map_count")]
public static int GetAddressMapCount(
    int interfaceId)
{
    return Interfaces[interfaceId]
        .GetAddressMapToRegister()
        .Count;
}
```

---

# Python Semantic Reconstruction

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

# Runtime Bootstrap Strategy

Application startup performs:

```python
import runtime.runtime_patch
import runtime.extension_methods
```

which:

* patches generated classes
* injects extension methods
* initializes registries
* loads CPython extension module

---

# Skill File Strategy

# File

```text
skill/generate_python_bindings.skill.md
```

---

# Skill Responsibilities

The skill generates:

* projection exports
* C++ bridge code
* Python extension module code
* runtime patch code
* semantic wrappers
* iterable adapters
* dictionary adapters
* extension injection code

---

# Skill Rules

## Generated Models

Do NOT modify:

* generated Python classes
* generated C# classes

Interop logic must remain external.

---

## Export Rules

Allowed export types:

```text
int
long
double
char*
handle
```

Disallowed:

```text
managed collections
managed objects
generic types
```

---

## Collection Projection Rules

### List<T>

Generate:

```text
count()
item_at(index)
```

### Dictionary<K,V>

Generate:

```text
count()
key_at(index)
value_at(index)
```

### IEnumerable<T>

Generate:

```text
iterator_create()
iterator_next()
iterator_current()
iterator_destroy()
```

or indexed projection when feasible.

---

## Semantic Wrapper Rules

Generate Python wrappers implementing:

* `__len__`
* `__getitem__`
* `__iter__`
* dict semantics
* iterable semantics

---

## Runtime Injection Rules

Generate:

```python
ClassName.extension_method = method
```

instead of modifying generated model source files.

---

## Performance Rules

Avoid:

* JSON serialization
* reflection
* CLR hosting
* object duplication

Prefer:

* handles
* projections
* lazy traversal
* iterable adapters

---

# Performance Analysis

## Advantages

| Area                  | Benefit   |
| --------------------- | --------- |
| Serialization         | Avoided   |
| Reflection            | Avoided   |
| CLR hosting           | Avoided   |
| Object duplication    | Avoided   |
| Identity preservation | Preserved |
| Scalability           | High      |
| Traversal overhead    | Low       |

---

# Risks

| Risk                            | Mitigation            |
| ------------------------------- | --------------------- |
| ABI complexity                  | Generated projections |
| Collection semantics complexity | Generated wrappers    |
| Memory ownership                | Handles only          |
| Iterator lifetime               | Runtime registry      |
| Large graph traversal           | Lazy traversal APIs   |

---

# Future Improvements

## Planned Optimizations

* cached projections
* lazy iterators
* incremental synchronization
* generated iterable adapters
* generated dictionary views
* generated collection semantics
* generated proxy views
* automatic iterator lifetime management

---

# Alternatives Considered

## JSON Serialization

Rejected due to:

* identity loss
* performance cost
* duplication

---

## pythonnet

Rejected due to:

* CLR hosting
* reflection overhead
* runtime coupling

---

## gRPC / REST

Rejected due to:

* excessive overhead
* in-process inefficiency

---

# Consequences

The system becomes:

```text
a generated language binding architecture
```

instead of:

* simple scripting bridge
* object serialization system

This architecture aligns with:

* CAD SDKs
* EDA SDKs
* LLVM bindings
* Qt/PyQt
* Vulkan
* industrial interoperability systems

---

# Final Decision

Adopt:

```text
Handle + Projection + Semantic Reconstruction Architecture
```

for all future Python ↔ C# interoperability.

Generated models remain untouched.

Interop behavior is:

* injected dynamically
* generated automatically
* projection-based
* ABI-safe
* scalable for large EDA systems

```
```
