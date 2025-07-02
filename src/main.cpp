#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <type_traits>
#include "overload.hpp"
#include "variant.hpp"


int visit_pure() {
    Variant<int, float, double> var;
    var.set<int>(2);

    auto &z = var;

    auto overloads = make_overload(
        [](const float &y) -> int { std::cout << "float " << y << "\n"; return 1; },
        [](const double &y) -> int { std::cout << "double " << y << "\n"; return 1; },
        [](const int &k) -> int { std::cout << "int " << k << "\n"; return k;});

    return z.visit(overloads);
}

int visit_mutating() {
    Variant<int, float, double> var;
    var.set<int>(2);

    auto &z = var;

    auto overloads = make_overload(
        [](float &y) -> int { std::cout << "float " << y << "\n"; return 1; },
        [](double &y) -> int { std::cout << "double " << y << "\n"; return 1; },
        [](int &k) -> int { std::cout << "int " << k++ << "\n"; return ++k;});

    return var.visit(overloads);
}


int main(int argc, char *argv[]) {

    constexpr int arr[] = {10, 20, 30, 40};

    static_assert(TypeIndex<int, float, int, double>::value == 1 && "Int at 0");

    int pure = visit_pure();
    int mutating = visit_mutating();

    std::cout << "Pure : " << pure << "\n";
    std::cout << "Mutating : " << mutating << "\n";


    return 0;
}
