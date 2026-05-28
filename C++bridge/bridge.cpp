# bridge.h

```cpp id="2v8s7n"
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

```cpp id="3w9x1y"
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

