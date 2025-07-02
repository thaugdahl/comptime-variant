#ifndef OVERLOAD_HPP
#define OVERLOAD_HPP
#include <utility>

//===------------------------------------------------------------===//
//=== Overload Set
//===------------------------------------------------------------===//


// C++ > 17, using ... pack expansion
template <typename... Fs>
struct overload_set : Fs... {
    using Fs::operator()...;

    overload_set(Fs &&...fs) : Fs(std::forward<Fs>(fs))... {}
};

// Pre C++-17
// -----------------------------------------------
// template <typename... Fns> struct overload_set;
// template <typename F1, typename... Fs>
// struct overload_set<F1, Fs...> : F1, overload_set<Fs...> {
//   using F1::operator();
//   using overload_set<Fs...>::operator();
//
//   overload_set(F1 &&f1, Fs &&...fs) noexcept(
//       std::is_nothrow_move_constructible_v<F1> &&
//       std::is_nothrow_move_constructible_v<overload_set<Fs...>>)
//       : F1(std::forward<F1>(f1)), overload_set<Fs...>(std::forward<Fs>(fs)...) {
//   }
// };
//
// template <typename F1> struct overload_set<F1> : F1 {
//   using F1::operator();
//
//   overload_set(F1 &&f1) noexcept(std::is_nothrow_move_constructible_v<F1>)
//       : F1(std::forward<F1>(f1)) {}
// };

template <typename... Fs> overload_set<Fs...> make_overload(Fs &&...fs) {
  return overload_set<Fs...>(std::forward<Fs>(fs)...);
}

#endif
