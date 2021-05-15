#include <iostream>
#include <string>

#include "constexpr_doublearray.hpp"

using namespace std::string_view_literals;

// Input words concatenated by NULL character (in string_view)
constexpr auto text = "ICDE\0"
                      "ICDM\0"
                      "ICDMW\0"
                      "ICML\0"
                      "SIGIR\0"
                      "SIGMOD\0"sv;

// Convert the text to the array of words
constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);

// Double-array dictionary
constexpr auto capacity = constexpr_doublearray::get_capacity(words);
constexpr auto dict = constexpr_doublearray::make<capacity>(words);

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

    std::cout << "decode(sig_ps[0]) = " << std::string(std::begin(sig0_dec), std::end(sig0_dec)) << std::endl;
    std::cout << "decode(sig_ps[1]) = " << std::string(std::begin(sig1_dec), std::end(sig1_dec)) << std::endl;

    return 0;
}