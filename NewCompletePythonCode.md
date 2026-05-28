# Complete Python Runtime With Automatic Object Binding

# Folder Structure

```text id="1a2b3c"
python/
│
├── generated/
│   ├── interface_definition.py
│   └── register_definition.py
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
├── EdaModel.dll
│
└── user_script.py
```

---

# generated/interface_definition.py

```python id="4d5e6f"
class InterfaceDefinition:

    def __init__(self):

        self.registers = []
```

---

# generated/register_definition.py

```python id="7g8h9i"
class RegisterDefinition:

    def __init__(self):

        self.name = ""

        self.offset = 0
```

---

# runtime/interop_registry.py

```python id="0j1k2l"
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

def get_handle(
    python_object):

    return _object_to_handle[
        python_object
    ]

def get_python_object(
    handle):

    return _handle_to_object[
        handle
    ]
```

---

# runtime/object_factory.py

```python id="3m4n5o"
from runtime.interop_registry \
    import bind_object

_next_handle = 1


def allocate_handle():

    global _next_handle

    value = _next_handle

    _next_handle += 1

    return value


def create_object(cls):

    obj = cls()

    handle = allocate_handle()

    bind_object(
        obj,
        handle)

    return obj
```

---

# runtime/runtime_patch.py

```python id="6p7q8r"
from generated.interface_definition \
    import InterfaceDefinition

from generated.register_definition \
    import RegisterDefinition

from runtime.interop_registry \
    import bind_object


_next_handle = 1


def allocate_handle():

    global _next_handle

    value = _next_handle

    _next_handle += 1

    return value


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


patch_constructor(
    InterfaceDefinition)

patch_constructor(
    RegisterDefinition)
```

---

# runtime/native_loader.py

```python id="9s0t1u"
import eda_native
```

---

# runtime/extension_methods.py

```python id="2v3w4x"
import eda_native

from generated.interface_definition \
    import InterfaceDefinition

from runtime.interop_registry \
    import get_handle

from runtime.interop_registry \
    import get_python_object


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


InterfaceDefinition \
    .get_address_map_to_register = (
        get_address_map_to_register
)
```

---

# bootstrap/initialize_runtime.py

```python id="5y6z7a"
import runtime.native_loader

import runtime.runtime_patch

import runtime.extension_methods
```

---

# user_script.py

```python id="8b9c0d"
from bootstrap.initialize_runtime import *

from generated.interface_definition \
    import InterfaceDefinition

from generated.register_definition \
    import RegisterDefinition


interface = InterfaceDefinition()

register1 = RegisterDefinition()
register1.name = "CTRL"
register1.offset = 0

register2 = RegisterDefinition()
register2.name = "STATUS"
register2.offset = 4


interface.registers.append(
    register1)

interface.registers.append(
    register2)


address_map = (
    interface
        .get_address_map_to_register()
)


for offset, register in (
        address_map.items()):

    print(
        offset,
        register.name)
```

---

# Final User Experience

User only writes:

```python id="1e2f3g"
interface.get_address_map_to_register()
```

No:

* bind_object
* handles
* ctypes
* bridge logic
* CLR knowledge
* serialization logic

---

# Runtime Flow

```text id="4h5i6j"
Object Creation
        ↓
Constructor Patch
        ↓
Automatic Handle Allocation
        ↓
Automatic Runtime Registration
        ↓
Extension Method Invocation
        ↓
CPython Extension
        ↓
C++ Bridge
        ↓
NativeAOT C#
        ↓
Semantic Reconstruction
        ↓
Natural Python Dictionary
```

---

# Important Architectural Benefit

Generated UML classes remain:

* pure
* generator-owned
* interop unaware

while runtime dynamically injects:

* identity tracking
* extension methods
* semantic reconstruction
* ABI interop behavior

```
```
