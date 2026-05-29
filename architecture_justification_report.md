# Architecture Justification Report
## NativeAOT + C++ Bridge + CPython Runtime vs IronPython

---

# 1. Objective

This document compares two approaches for integrating Python with C# generated models:

- IronPython (https://ironpython.net)
- NativeAOT + C++ Bridge + CPython Runtime (Our Design)

Goal is to enable natural Python APIs like:

```python
interface.get_address_map_to_register()
```

while executing logic in C#.

---

# 2. Architecture Overview

## 2.1 IronPython

```text
Python Code
    ↓
IronPython Runtime
    ↓
CLR (.NET Runtime)
    ↓
C# Assemblies
```

Python runs inside the .NET runtime.

---

## 2.2 Our Design

```text
Python (CPython)
    ↓
CPython Extension (.pyd)
    ↓
C++ Bridge
    ↓
NativeAOT C# Library
```

Python and .NET are separate runtimes connected via native ABI.

---

# 3. Key Comparison

## 3.1 Python Ecosystem Support

| Feature | IronPython | Our Design |
|--------|------------|------------|
| NumPy / SciPy | ❌ Limited | ✅ Full support |
| Pandas | ❌ Issues | ✅ Works natively |
| PyTorch / TensorFlow | ❌ Not supported | ✅ Supported |
| C-extension packages | ❌ Not compatible | ✅ Fully compatible |

---

## 3.2 Runtime Model

### IronPython

Python runs inside CLR (single runtime).

### Our Design

Python ↔ Native ABI ↔ .NET Runtime

Independent runtimes with a strict ABI boundary.

---

## 3.3 Interoperability Approach

### IronPython
- CLR reflection-based invocation
- Tight runtime coupling

### Our Design
- NativeAOT exports
- C ABI boundary
- C++ bridge layer

Benefits:
- stable binary contracts
- language-independent boundary
- explicit API control

---

## 3.4 Deployment Model

### IronPython
Requires:
- IronPython runtime
- .NET runtime

### Our Design
Requires only:
- eda_native.pyd
- ConvenienceApiLib.dll

Lightweight deployment with no CLR hosting in Python.

---

## 3.5 Generated API Scalability

### IronPython
- Suitable for small scripts
- Limited scalability for large generated systems

### Our Design
- Designed for UML-generated large systems
- Supports extension methods + projections
- Scales to large object graphs

---

## 3.6 Object Identity Model

### IronPython
- CLR-managed object identity
- Shared runtime model

### Our Design
Python Object ↔ Handle ↔ C# Object

Explicit identity mapping with no duplication.

---

## 3.7 Performance Model

### IronPython
- CLR reflection-based dispatch

### Our Design
- Native ABI calls
- NativeAOT compiled execution
- Minimal overhead

---

# 4. Summary

| Category | IronPython | Our Design |
|----------|------------|------------|
| Python compatibility | ❌ Partial | ✅ Full CPython |
| Runtime coupling | ❌ Tight | ✅ Loose |
| Performance | ⚖ Medium | ✅ High |
| Ecosystem support | ❌ Limited | ✅ Full |
| Generated API support | ⚖ Moderate | ✅ Strong |
| Deployment | ❌ Heavy | ✅ Lightweight |

---

# 5. Conclusion

We choose:

CPython + NativeAOT + C++ Bridge

Because it provides:
- full Python ecosystem support
- runtime isolation
- scalable generated APIs
- stable ABI-based communication
- better performance
- simpler deployment

---

# Final Statement

IronPython is suitable for embedded scripting inside .NET.

Our design is suitable for large-scale generated systems and high-performance interoperability.

