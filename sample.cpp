#include <iostream>

#include "constexpr_doublearray.hpp"

using namespace std::string_view_literals;

// Input words in text format
constexpr auto text = "ICDE\0ICDM\0ICDMW\0ICML\0SIGIR\0SIGMOD\0"sv;

// Text -> Array<Word>
constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);

// Routine for constructing the double-array dictionary.
constexpr auto make_doublearray() {
    constexpr auto capacity = constexpr_doublearray::get_capacity(words);
    constexpr auto units = constexpr_doublearray::make<capacity>(words);
    return constexpr_doublearray::utils::shrink<std::size(units)>(units);
}

// Double-array dictionary
constexpr auto dict = make_doublearray();

// Exact search
constexpr auto icde_sr = constexpr_doublearray::search("ICDE"sv, dict);
constexpr auto sigmod_sr = constexpr_doublearray::search("SIGMOD"sv, dict);
constexpr auto sigkdd_sr = constexpr_doublearray::search("SIGKDD"sv, dict);

// Common prefix search
constexpr auto icdmw_cpsr = constexpr_doublearray::common_prefix_search<5>("ICDMW"sv, dict);

// Predictive search
constexpr auto sig_ps = constexpr_doublearray::predictive_search<2>("SIG"sv, dict);

// Decode the stored words
constexpr auto sig0_dec = constexpr_doublearray::decode<sig_ps[0].depth>(sig_ps[0].npos, dict);
constexpr auto sig1_dec = constexpr_doublearray::decode<sig_ps[1].depth>(sig_ps[1].npos, dict);

int main() {
    std::cout << "search(ICDE) = " << icde_sr.id << std::endl;
    std::cout << "search(SIGMOD) = " << sigmod_sr.id << std::endl;
    std::cout << "search(SIGKDD) = " << sigkdd_sr.id << std::endl;

    std::cout << "common_prefix_search(ICDMW) = (";
    for (auto r : icdmw_cpsr) std::cout << r.id << ",";
    std::cout << ")" << std::endl;

    std::cout << "predictive_search(SIG) = (";
    for (auto r : sig_ps) std::cout << r.id << ",";
    std::cout << ")" << std::endl;

    std::cout << "decode(predictive_search(SIG)[0]) = ";
    for (auto c : sig0_dec) std::cout << c;
    std::cout << std::endl;

    std::cout << "decode(predictive_search(SIG)[1]) = ";
    for (auto c : sig1_dec) std::cout << c;
    std::cout << std::endl;

    return 0;
}