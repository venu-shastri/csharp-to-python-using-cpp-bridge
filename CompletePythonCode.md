# Complete Python Project

# Folder Structure

```text id="8p4rty"
python/
│
├── generated/
│   ├── interface_definition.py
│   └── register_definition.py
│
├── runtime/
│   ├── interop_registry.py
│   ├── extension_methods.py
│   └── native_loader.py
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

```python id="3k7nfa"
class InterfaceDefinition:

    def __init__(self):

        self.registers = []
```

---

# generated/register_definition.py

```python id="9wq5db"
class RegisterDefinition:

    def __init__(self):

        self.name = ""

        self.offset = 0
```

---

# runtime/interop_registry.py

```python id="1z3x5c"
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

# runtime/native_loader.py

```python id="7v8b6n"
import eda_native
```

---

# runtime/extension_methods.py

```python id="2m4q8k"
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

```python id="5c7v9b"
import runtime.native_loader

import runtime.extension_methods
```

---

# user_script.py

```python id="8n2m6k"
from bootstrap.initialize_runtime import *

from generated.interface_definition \
    import InterfaceDefinition

from generated.register_definition \
    import RegisterDefinition

from runtime.interop_registry \
    import bind_object


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


#
# Bind Python objects to native handles
#
# Normally auto-generated runtime code
# would do this automatically.
#

bind_object(interface, 1)

bind_object(register1, 100)

bind_object(register2, 101)


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

# Runtime Flow

```text id="4f6g8h"
user_script.py
        ↓
generated model object
        ↓
runtime extension method
        ↓
eda_native.pyd
        ↓
C++ bridge
        ↓
NativeAOT DLL
        ↓
C# extension method
        ↓
projection APIs
        ↓
semantic reconstruction
        ↓
Python dictionary
```

---

# Important Note

This example manually calls:

```python id="1h3j5k"
bind_object(...)
```

In your actual architecture this should be:

* generated automatically,
* hidden from users,
* initialized during runtime bootstrap.

User should ONLY see:

```python id="7l9m2n"
interface.get_address_map_to_register()
```

---

# Final User Experience

```python id="6p8q1r"
result = (
    interface
        .get_address_map_to_register()
)
```

without:

* JSON,
* ctypes,
* native handles,
* bridge knowledge,
* CLR hosting,
* serialization.

```
```
