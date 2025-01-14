# Variant Type with Compile-Time Dispatch Tables

This document outlines the implementation of a **Variant** type with **compile-time computed dispatch tables**. A Variant type is a tagged union that allows efficient type-safe storage and manipulation of multiple types, with the added benefit of compile-time dispatch for operations.

---

## Overview

### Tagged Union Example
A traditional union in C++ does not track the active type. For example:

```cpp
struct K {
    union {
        int;
        float;
        long;
        std::string;
    } A;
};
```

To track the active type, we must add an active index:

```cpp
struct K {
    union {
        int;
        float;
        long;
        std::string;
    } A;
    size_t active_index; // Tracks the active type
};
```

Using this `active_index`, we can create a dispatch table to invoke the correct function for the stored type:

```cpp
using VisitorType = SomeInvokableType;

void intFunc(const void *data, VisitorType visitor) {
    visitor(*reinterpret_cast<const int *>(data));
}

void floatFunc(const void *data, VisitorType visitor) {
    visitor(*reinterpret_cast<const float *>(data));
}

// ...

std::array<void (*)(const void *, VisitorType), 4> dispatchTable{
    intFunc,
    floatFunc,
    longFunc,
    stringFunc
};
```

This approach works but requires manual management. Instead, we will implement a **Variant** type that automates this process.

---

## Implementation

### Type Utilities

#### **TypeIndex**
`TypeIndex` computes the index of a type in a parameter pack at compile time.

```cpp
template <typename... Types>
struct TypeIndex;

template <typename T, typename First, typename... Rest>
struct TypeIndex<T, First, Rest...> {
    static constexpr size_t value = 0;
};

template <typename T, typename First, typename... Rest>
struct TypeIndex<T, First, Rest...> {
    static constexpr size_t value = 1 + TypeIndex<T, Rest...>::value;
};
```

Example:
```cpp
constexpr size_t index = TypeIndex<int, float, std::string, int, long>::value; // index = 2
```

#### **TypeAt**
`TypeAt` retrieves the type at a given index in a parameter pack.

```cpp
template <size_t Index, typename... Types>
struct TypeAt;

template <typename First, typename... Rest>
struct TypeAt<0, First, Rest...> {
    using type = First;
};

template <size_t Index, typename First, typename... Rest>
struct TypeAt<Index, First, Rest...> {
    using type = typename TypeAt<Index - 1, Rest...>::type;
};
```

Example:
```cpp
using T = TypeAt<2, int, float, std::string>::type; // T = std::string
```

---

### Variant Class

#### **Storage and Alignment**
The `Variant` class must allocate enough space for the largest type and ensure proper alignment.

```cpp
template <class... Types>
class Variant {
private:
    static constexpr std::size_t MaxSize = std::max({sizeof(Types)...});
    static constexpr std::size_t MaxAlign = std::max({alignof(Types)...});

    using Storage = std::byte[MaxSize];

    alignas(MaxAlign) Storage data; // Storage for the current object
    size_t active_index;            // Index of the currently stored type

public:
    Variant() : active_index(sizeof...(Types)) {} // Default constructor
};
```

#### **Destructor**
The destructor must destroy the currently active object.

```cpp
template <class... Types>
class Variant {
private:
    template <size_t Index = 0>
    void destroy() {
        if (active_index == Index) {
            using ActiveType = typename TypeAt<Index, Types...>::type;
            reinterpret_cast<ActiveType *>(&data)->~ActiveType();
        } else if constexpr (Index + 1 < sizeof...(Types)) {
            destroy<Index + 1>();
        }
    }

public:
    ~Variant() { destroy(); }
};
```

#### **Setter**
The `set` method assigns a new value to the `Variant`.

```cpp
template <typename T>
void set(T &&value) {
    destroy(); // Destroy previously stored object
    new (&data) T(std::forward<T>(value)); // Placement new to construct in-place
    active_index = TypeIndex<T, Types...>::value; // Update active index
}
```

#### **Visitor**
The `visit` method applies an invokable visitor to the stored object.

```cpp
template <typename Visitor>
void visit(Visitor &&visitor) const {
    static constexpr auto dispatch_table = make_dispatch_table<Visitor>();
    dispatch_table[active_index](&data, visitor);
}
```

---

### Dispatch Table

The dispatch table is a compile-time array of function pointers that recast the stored data and invoke the visitor.

#### **Implementation**
We use stateless lambdas and parameter pack expansion to generate the table.

```cpp
template <typename Visitor, size_t... Indices>
static constexpr auto make_dispatch_table_impl(std::index_sequence<Indices...>) {
    return std::array<void (*)(const void *, Visitor &), sizeof...(Types)>{
        [](const void *data_ptr, Visitor &visitor) {
            using CurrentType = typename TypeAt<Indices, Types...>::type;
            visitor(*reinterpret_cast<const CurrentType *>(data_ptr));
        }...
    };
}

template <typename Visitor>
static constexpr auto make_dispatch_table() {
    return make_dispatch_table_impl<Visitor>(std::make_index_sequence<sizeof...(Types)>());
}
```

---

## Summary

This implementation provides a robust `Variant` type with:

- Automatic tracking of the active index.
- Compile-time computed dispatch tables for efficient runtime operations.
- Support for custom visitors using argument-dependent lookup (ADL).

This design ensures type safety and efficiency while leveraging modern C++ features like template metaprogramming and constexpr.
