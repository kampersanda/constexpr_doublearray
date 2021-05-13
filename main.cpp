#include <iostream>

#include "constexpr_doublearray.hpp"

using namespace std::string_view_literals;

static constexpr auto text = "abc\0abd\0bcc\0ccsa\0"sv;

std::ostream& operator<<(std::ostream& os, const std::tuple<std::size_t, std::size_t>& v) {
    os << std::get<0>(v) << ',' << std::get<1>(v);
    return os;
}

constexpr auto make_units() {
    constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
    constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);
    constexpr auto capacity = constexpr_doublearray::get_capacity(words);
    constexpr auto units = constexpr_doublearray::make<capacity>(words);
    constexpr auto size = constexpr_doublearray::get_size(units);
    return constexpr_doublearray::shrink_to_fit<size>(units);
}

int main() {
    constexpr auto units = make_units();

    std::cout << constexpr_doublearray::search(units, "abc"sv) << std::endl;
    std::cout << constexpr_doublearray::search(units, "abd"sv) << std::endl;
    std::cout << constexpr_doublearray::search(units, "bcc"sv) << std::endl;
    std::cout << constexpr_doublearray::search(units, "ccsa"sv) << std::endl;
    std::cout << constexpr_doublearray::search(units, "ab"sv) << std::endl;

    return 0;
}