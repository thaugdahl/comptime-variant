#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
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
  Variant() : active_index(-1) {}

  ~Variant() { destroy(); }

  template <typename T> void set(T &&value) {
    destroy();
    // Placement new
    new (&data) T(std::forward<T>(value));
    active_index = TypeIndex<T, Types...>::value;
  }

  template <typename Visitor> void visit(Visitor &&visitor) const {
    static constexpr auto dispatch_table = make_dispatch_table<Visitor>();
    dispatch_table[active_index](&data, visitor);
  }

private:
  template <typename Visitor, size_t... Indices>
  static constexpr auto make_dispatch_table_impl(index_sequence<Indices...>) {
    return std::array<void (*)(const void *, Visitor &), sizeof...(Types)>{
        [](const void *data_ptr, Visitor &visitor) {
          using CurrentType = typename TypeAt<Indices, Types...>::type;
          visitor(*reinterpret_cast<const CurrentType *>(data_ptr));
        }...};
  }

  template <typename Visitor> static constexpr auto make_dispatch_table() {
    return make_dispatch_table_impl<Visitor>(
        make_index_sequence<sizeof...(Types)>());
  }
};

template <typename... Fns> struct overload_set;

template <typename F1, typename... Fs>
struct overload_set<F1, Fs...> : F1, overload_set<Fs...> {
  using F1::operator();
  using overload_set<Fs...>::operator();

  overload_set(F1 &&f1, Fs &&...fs) noexcept(
      std::is_nothrow_move_constructible_v<F1> &&
      std::is_nothrow_move_constructible_v<overload_set<Fs...>>)
      : F1(std::forward<F1>(f1)), overload_set<Fs...>(std::forward<Fs>(fs)...) {
  }
};

template <typename F1> struct overload_set<F1> : F1 {
  using F1::operator();

  overload_set(F1 &&f1) noexcept(std::is_nothrow_move_constructible_v<F1>)
      : F1(std::forward<F1>(f1)) {}
};

template <typename... Fs> overload_set<Fs...> make_overload(Fs &&...fs) {
  return overload_set<Fs...>(std::forward<Fs>(fs)...);
}

int main(int argc, char *argv[]) {

  constexpr int arr[] = {10, 20, 30, 40};

  static_assert(TypeIndex<float, int, float>::value == 1 && "Int at 0");

  Variant<int, std::string, std::vector<int>> var;

  std::cout << "\n";
  std::cout << "\n";
  std::vector<int> k{1, 2, 3, 4};

  var.set(std::move(k));

  auto overloads = make_overload(
      [](const std::vector<int> &v) {
        std::cout << "Outputting a list \n";
        for (auto &s : v) {
          std::cout << s << "\n";
        }
      },
      [](int k) { std::cout << "int " << k << "\n"; },
      [](const std::string &v) { std::cout << "string " << v << "\n"; });

  var.visit(overloads);

  std::cout << "Hello world\n";

  return 0;
}
