#include <iostream>

#include "constexpr_doublearray.hpp"
#include "words.txt.hpp"

// Text -> Array<Word>
constexpr auto num_words = constexpr_doublearray::utils::get_num_words(text);
constexpr auto words = constexpr_doublearray::utils::text_to_words<num_words>(text);

// Double-array dictionary
constexpr auto capacity = constexpr_doublearray::get_capacity(words);
constexpr auto dict = constexpr_doublearray::make<capacity>(words);

int main() {
    std::string q;
    std::cin >> q;

    size_t res_id = constexpr_doublearray::search(q, dict).id;
    std::cout << res_id << std::endl;

    return 0;
}