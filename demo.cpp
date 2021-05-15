#include <iostream>
#include <string>

#include "constexpr_doublearray.hpp"
#include "dataset.hpp"

// Text -> Array<Word>
constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);

// Double-array dictionary
constexpr auto capacity = constexpr_doublearray::get_capacity(words);
constexpr auto dict = constexpr_doublearray::make<capacity>(words);

// Buffer sizes for search results
constexpr auto decode_size = constexpr_doublearray::utils::get_max_length(words);
constexpr auto predictive_size = static_cast<std::size_t>(20);

template <class Vec>
auto vec_to_str(const Vec& vec) {
    return std::string(std::begin(vec), std::end(vec));
}

int main() {
    std::cout << "Query modes:" << std::endl;
    std::cout << " - 1: search" << std::endl;
    std::cout << " - 2: common_prefix_search" << std::endl;
    std::cout << " - 3: predictive_search" << std::endl;
    std::cout << " - others: exit" << std::endl;

    int qmode = 0;
    std::string qword;

    while (true) {
        std::cout << "Input query mode (int):" << std::endl;
        std::cout << "> ";
        std::cin >> qmode;

        if (qmode < 1 or 3 < qmode) {
            std::cout << "Good bye!!" << std::endl;
            break;
        }

        std::cout << "Input query word (string):" << std::endl;
        std::cout << "> ";
        std::cin >> qword;

        if (qmode == 1) {
            const auto res = constexpr_doublearray::search(qword, dict);
            if (res.id == constexpr_doublearray::NOT_FOUND) {
                std::cout << "Not found" << std::endl;
            } else {
                std::cout << "id = " << res.id << std::endl;
            }
        } else if (qmode == 2) {
            const auto res = constexpr_doublearray::common_prefix_search<decode_size>(qword, dict);
            if (std::size(res) == 0) {
                std::cout << "Not found" << std::endl;
            } else {
                for (std::size_t i = 0; i < std::size(res); i++) {
                    const auto dec = constexpr_doublearray::decode<decode_size>(res[i].npos, dict);
                    std::cout << i + 1 << ": "
                              << "id = " << res[i].id << ", str = " << vec_to_str(dec) << std::endl;
                }
            }
        } else if (qmode == 3) {
            const auto res = constexpr_doublearray::predictive_search<predictive_size>(qword, dict);
            if (std::size(res) == 0) {
                std::cout << "Not found" << std::endl;
            } else {
                for (std::size_t i = 0; i < std::size(res); i++) {
                    const auto dec = constexpr_doublearray::decode<decode_size>(res[i].npos, dict);
                    std::cout << i + 1 << ": "
                              << "id = " << res[i].id << ", str = " << vec_to_str(dec) << std::endl;
                }
            }
        }
    }

    return 0;
}
