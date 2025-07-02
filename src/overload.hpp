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

template <typename... Fs> overload_set<Fs...> make_overload(Fs &&...fs) {
  return overload_set<Fs...>(std::forward<Fs>(fs)...);
}

#endif
