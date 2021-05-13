#include <iostream>

#include "constexpr_doublearray.hpp"

using namespace std::string_view_literals;

int main() {
    constexpr auto text = "abc\0abd\0bcc\0ccsa\0"sv;

    constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
    constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);

    constexpr auto capacity = constexpr_doublearray::get_capacity(words);
    constexpr auto units = constexpr_doublearray::make<capacity>(words);

    std::cout << constexpr_doublearray::search(units, "abc"sv) << std::endl;
    std::cout << constexpr_doublearray::search(units, "abd"sv) << std::endl;
    std::cout << constexpr_doublearray::search(units, "bcc"sv) << std::endl;

    return 0;
}