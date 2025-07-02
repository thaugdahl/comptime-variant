#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <algorithm>
#include <cstddef>
#include "traits.hpp"
#include <array>

//===------------------------------------------------------------====//
//=== Variant Implementation
//===------------------------------------------------------------====//


template <class... Types> class Variant;

struct VariantDispatchTable {

private:
    template <class Visitor, class RetT, class... Types, std::size_t... Indices>
    static constexpr auto get_impl(index_sequence<Indices...>) {
        return std::array<RetT (*)(const void *, Visitor &), sizeof...(Types)>{
            [](void *data_ptr, Visitor &visitor) {
                using CurrentType = typename TypeAt<Indices, Types...>::type;
                return visitor(*reinterpret_cast<const CurrentType *>(data_ptr));
            }...};
    }

    template <class Visitor, class Ret, class... Types>
    static constexpr auto get() {
        return get_impl<Visitor, Ret, Types...>(make_index_sequence<sizeof...(Types)>());
    };

    template <class... Types>
    friend class Variant;

};


struct VariantDispatchTableMut {
private:
    template <class Visitor, class RetT, class... Types, std::size_t... Indices>
    static constexpr auto get_impl(index_sequence<Indices...>) {
        return std::array<RetT (*)(void *, Visitor &), sizeof...(Types)>{
            [](void *data_ptr, Visitor &visitor) {
                using CurrentType = typename TypeAt<Indices, Types...>::type;
                return visitor(*reinterpret_cast<CurrentType *>(data_ptr));
            }...};
    }

    template <class Visitor, class Ret, class... Types>
    static constexpr auto get() {
        return get_impl<Visitor, Ret, Types...>(make_index_sequence<sizeof...(Types)>());
    }

    template <class... Types>
    friend class Variant;
};



template <typename... Types> class Variant {
    static constexpr std::size_t MaxSize = std::max({sizeof(Types)...});
    static constexpr std::size_t MaxAlign = std::max({alignof(Types)...});
    using Storage = std::byte[MaxSize];

    alignas(MaxAlign) Storage data;
    size_t active_index;

    template <size_t Index = 0> void destroy() {
        if (active_index == Index) {
            using ActiveType = typename TypeAt<Index, Types...>::type;
            reinterpret_cast<ActiveType *>(&data)->~ActiveType();
        } else if constexpr (Index + 1 < sizeof...(Types)) {
            destroy<Index + 1>();
        }
    }

public:
    Variant() : active_index{sizeof...(Types)} {}

    ~Variant() { destroy(); }

    template <typename T> void set(T &&value) {
        destroy();
        // Placement new
        new (&data) T(std::forward<T>(value));
        active_index = TypeIndex<T, Types...>::value;
    }

    using FirstTypeT = typename TypeAt<0, Types...>::type;

    template <class Visitor, class Type>
    static constexpr void validate_return_types() {
        using FirstReturn = decltype(std::declval<Visitor>()(std::declval<Type &>()));
        constexpr bool all_eq = all_equal<FirstReturn, decltype(std::declval<Visitor>()(std::declval<Types &>()))...>::value;
        static_assert(all_eq, "Your overload set must have consistent return types!");
    }

    // ======================================
    // MUTATING VISITOR
    // ======================================
    template <typename Visitor> decltype(auto) visit(Visitor &&visitor) {
        (void) validate_return_types<Visitor, FirstTypeT>();
        using RetT = typename SingleArgReturnType<Visitor, FirstTypeT &>::type;
        static constexpr auto dispatch_table = VariantDispatchTableMut::get<Visitor, RetT, Types...>();
        return dispatch_table[active_index](&data, visitor);
    }

    // ======================================
    // PURE VISITOR
    // ======================================
    template <typename Visitor> decltype(auto) visit(Visitor &&visitor) const {
        (void)validate_return_types<Visitor, const FirstTypeT>();

        using RetT = typename SingleArgReturnType<Visitor, FirstTypeT &>::type;
        static constexpr auto dispatch_table = VariantDispatchTable::get<Visitor, RetT, Types...>();
        return dispatch_table[active_index](&data, visitor);
    }

};

#endif
