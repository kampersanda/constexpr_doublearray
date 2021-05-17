#include <iostream>
#include <string>
#include <vector>

#include <constexpr_doublearray.hpp>

using namespace std::string_view_literals;

int main() {
    // Input words concatenated by NULL character (must be sorted)
    const auto text = "ICDE\0"
                      "ICDM\0"
                      "ICDMW\0"
                      "ICML\0"
                      "SIGIR\0"
                      "SIGMOD\0"sv;

    // Convert the text to the array of words
    const auto words = constexpr_doublearray::dynamic::utils::text_to_words(text);

    // Double-array dictionary
    const auto dict = constexpr_doublearray::dynamic::make(words);

    // Simple search
    const auto icde_sr = constexpr_doublearray::search("ICDE"sv, dict);
    const auto sigmod_sr = constexpr_doublearray::search("SIGMOD"sv, dict);
    const auto sigkdd_sr = constexpr_doublearray::search("SIGKDD"sv, dict);

    std::cout << "search(ICDE) = " << icde_sr.id << std::endl;
    std::cout << "search(SIGMOD) = " << sigmod_sr.id << std::endl;
    std::cout << "search(SIGKDD) = " << sigkdd_sr.id << std::endl;

    // Decoding
    const auto icde_dec = constexpr_doublearray::dynamic::decode(icde_sr.npos, dict);
    const auto sigmod_dec = constexpr_doublearray::dynamic::decode(sigmod_sr.npos, dict);

    std::cout << "decode(icde_sr) = " << std::string(std::begin(icde_dec), std::end(icde_dec)) << std::endl;
    std::cout << "decode(sigmod_sr) = " << std::string(std::begin(sigmod_dec), std::end(sigmod_dec)) << std::endl;

    // Common prefix search
    const auto icdmw_cpsr = constexpr_doublearray::dynamic::common_prefix_search("ICDMW"sv, dict);

    std::cout << "common_prefix_search(ICDMW) = ";
    for (auto r : icdmw_cpsr) {
        const auto dec = constexpr_doublearray::dynamic::decode(r.npos, dict);
        std::cout << "(id=" << r.id << ",str=" << std::string(std::begin(dec), std::end(dec)) << "), ";
    }
    std::cout << std::endl;

    // Predictive search
    const auto sig_psr = constexpr_doublearray::dynamic::predictive_search("SIG"sv, dict);

    std::cout << "predictive_search(SIG) = ";
    for (auto r : sig_psr) {
        const auto dec = constexpr_doublearray::dynamic::decode(r.npos, dict);
        std::cout << "(id=" << r.id << ",str=" << std::string(std::begin(dec), std::end(dec)) << "), ";
    }
    std::cout << std::endl;

    return 0;
}
