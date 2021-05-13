#include <iostream>

#include "constexpr_doublearray.hpp"

using namespace std::string_view_literals;

static constexpr std::array<std::string_view, 5> words = {
    "ICDE\0"sv, "ICDM\0"sv, "ICML\0"sv, "SIGIR\0"sv, "SIGMOD\0"sv,
};

std::ostream& operator<<(std::ostream& os, const std::tuple<std::size_t, std::size_t>& v) {
    os << std::get<0>(v) << ',' << std::get<1>(v);
    return os;
}

constexpr auto make_units() {
    constexpr auto capacity = constexpr_doublearray::get_capacity(words);
    constexpr auto units = constexpr_doublearray::make<capacity>(words);
    constexpr auto size = constexpr_doublearray::get_size(units);
    return constexpr_doublearray::shrink_to_fit<size>(units);
}

int main() {
    constexpr auto units = make_units();

    std::cout << constexpr_doublearray::search(units, "ICDE"sv) << std::endl;

    return 0;
}