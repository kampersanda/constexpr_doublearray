#include <iostream>
#include <string>

#include <constexpr_doublearray.hpp>

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

// Simple search
constexpr auto icde_sr = constexpr_doublearray::search("ICDE"sv, dict);
constexpr auto sigmod_sr = constexpr_doublearray::search("SIGMOD"sv, dict);
constexpr auto sigkdd_sr = constexpr_doublearray::search("SIGKDD"sv, dict);

// Decoding
constexpr auto icde_dec = constexpr_doublearray::decode<icde_sr.depth>(icde_sr.npos, dict);
constexpr auto sigmod_dec = constexpr_doublearray::decode<sigmod_sr.depth>(sigmod_sr.npos, dict);

// Common prefix search
constexpr auto icdmw_cpsr = constexpr_doublearray::common_prefix_search<5>("ICDMW"sv, dict);

// Predictive search
constexpr auto sig_psr = constexpr_doublearray::predictive_search<2>("SIG"sv, dict);

// Buffer size for decoding
constexpr auto decode_size = constexpr_doublearray::utils::get_max_length(words);

int main() {
    std::cout << "search(ICDE) = " << icde_sr.id << std::endl;
    std::cout << "search(SIGMOD) = " << sigmod_sr.id << std::endl;
    std::cout << "search(SIGKDD) = " << sigkdd_sr.id << std::endl;

    std::cout << "decode(icde_sr) = " << std::string(std::begin(icde_dec), std::end(icde_dec)) << std::endl;
    std::cout << "decode(sigmod_sr) = " << std::string(std::begin(sigmod_dec), std::end(sigmod_dec)) << std::endl;

    std::cout << "common_prefix_search(ICDMW) = ";
    for (auto r : icdmw_cpsr) {
        const auto dec = constexpr_doublearray::decode<std::size("ICDMW"sv)>(r.npos, dict);
        std::cout << "(id=" << r.id << ",str=" << std::string(std::begin(dec), std::end(dec)) << "), ";
    }
    std::cout << std::endl;

    std::cout << "predictive_search(SIG) = ";
    for (auto r : sig_psr) {
        const auto dec = constexpr_doublearray::decode<decode_size>(r.npos, dict);
        std::cout << "(id=" << r.id << ",str=" << std::string(std::begin(dec), std::end(dec)) << "), ";
    }
    std::cout << std::endl;

    return 0;
}
