#include <iostream>

#include "constexpr_doublearray.hpp"

using namespace std::string_view_literals;

constexpr auto text = "ICDE\0ICDM\0ICML\0SIGIR\0SIGMOD\0"sv;
constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);

// Routine for constructing the double array.
constexpr auto make_doublearray() {
    constexpr auto capacity = constexpr_doublearray::get_capacity(words);
    constexpr auto units = constexpr_doublearray::make<capacity>(words);
    return constexpr_doublearray::utils::shrink<std::size(units)>(units);
}

// Double-array dictionary
constexpr auto dict = make_doublearray();

//
constexpr auto icde_sr = constexpr_doublearray::search("ICDE"sv, dict);
constexpr auto sigmod_sr = constexpr_doublearray::search("SIGMOD"sv, dict);

constexpr auto icde_cpsr = constexpr_doublearray::common_prefix_search<4>("ICDE"sv, dict);

// // constexpr auto sigmod_ex = constexpr_doublearray::extract<sigmod_res.depth>(units);

constexpr auto icde_dec = constexpr_doublearray::decode<icde_sr.depth>(icde_sr.npos, dict);

std::ostream& operator<<(std::ostream& os, const constexpr_doublearray::search_result& v) {
    os << "id=" << v.id << ",npos=" << v.npos << ",depth=" << v.depth;
    return os;
}

template <class Vec>
void print_vec(const Vec& vec) {
    for (auto v : vec) {
        std::cout << v;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << icde_sr << std::endl;
    std::cout << sigmod_sr << std::endl;

    print_vec(icde_dec);

    return 0;
}