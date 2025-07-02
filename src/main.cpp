#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <type_traits>
#include "traits.hpp"
#include "overload.hpp"
#include "variant.hpp"



int main(int argc, char *argv[]) {

    constexpr int arr[] = {10, 20, 30, 40};

    static_assert(TypeIndex<int, float, int, double>::value == 1 && "Int at 0");

    Variant<int, float, double> var;

    var.set<int>(2);

    auto overloads = make_overload(
        [](float &y) -> int { std::cout << "float " << y << "\n"; return 1; },
        [](double &y) -> int { std::cout << "double " << y << "\n"; return 1; },
        [](int &k) -> int { std::cout << "int " << k++ << "\n"; return 2;});


    const auto &z = var;
    auto x = var.visit(overloads);

    (void)var.visit([](auto &c) { std::cout << "printing " <<  c << "\n"; } );

    std::cout << x << "\n";

    std::cout << "Hello world\n";

    return 0;
}
