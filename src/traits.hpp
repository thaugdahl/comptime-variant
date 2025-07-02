#ifndef TRAITS_HPP
#define TRAITS_HPP
///===------------------------------------------------------------===//
///=== Utilities and Type Traits
///===------------------------------------------------------------===//

#include <cstddef>
#include <type_traits>

template <std::size_t... Indices> struct index_sequence {};

template <std::size_t N, std::size_t... Indices>
struct make_index_sequence_impl
    : make_index_sequence_impl<N - 1, N - 1, Indices...> {};

template <std::size_t... Indices>
struct make_index_sequence_impl<0, Indices...> {
  using type = index_sequence<Indices...>;
};

template <std::size_t N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;

template <typename T, std::size_t... Indices>
void print_indices(const T &arr, index_sequence<Indices...>) {
  ((std::cout << arr[Indices] << " "), ...);
}

template <typename T, typename... Types> struct TypeIndex;

template <typename T, typename... Rest> struct TypeIndex<T, T, Rest...> {
  static constexpr size_t value = 0;
};

template <typename T, typename First, typename... Rest>
struct TypeIndex<T, First, Rest...> {
  static constexpr size_t value = 1 + TypeIndex<T, Rest...>::value;
};

template <typename...> using void_t = void;

template <typename T> struct TypeIndex<T> {
  static_assert(sizeof(T) == 0, "Type not found in variant!");
};

template <size_t Index, typename... Types> struct TypeAt;

template <typename First, typename... Rest> struct TypeAt<0, First, Rest...> {
  using type = First;
};

template <size_t Index, typename First, typename... Rest>
struct TypeAt<Index, First, Rest...> {
  using type = typename TypeAt<Index - 1, Rest...>::type;
};


template <class First, class... Others>
struct all_equal;

template <class First>
struct all_equal<First> {
    static constexpr bool value = true;
};

template <class First, class Second, class... Others>
struct all_equal<First, Second, Others...> {
    static constexpr bool value = std::is_same_v<First, Second> && all_equal<First, Others...>::value;
};


template <class Fn, class Arg>
struct SingleArgReturnType {
    using type = decltype(std::declval<Fn>()(std::declval<Arg>()));
};

#endif

// vim: set ft=cpp
