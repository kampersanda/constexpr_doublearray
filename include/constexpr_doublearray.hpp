#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "fixed_capacity_vector"

#ifdef CXPRDA_DISABLE_CONSTEXPR
#define CXPRDA_CONSTEXPR
#else
#define CXPRDA_CONSTEXPR constexpr
#endif

namespace constexpr_doublearray {

template <class T, std::size_t Capacity>
using static_vector = std::experimental::fixed_capacity_vector<T, Capacity>;

static constexpr double RESERVE_FACTOR = 1.1;
static constexpr char END_MARKER = '\000';
static constexpr auto NOT_FOUND = std::numeric_limits<std::size_t>::max();

namespace utils {

// Constexpr version of algorithm::find
template <class Iterator, class Value>
constexpr Iterator find(Iterator first, Iterator last, const Value& value) {
    for (; first != last; ++first) {
        if (*first == value) break;
    }
    return first;
}

// Constexpr version of algorithm::iter_swap
template <class Iterator>
constexpr void iter_swap(Iterator a, Iterator b) {
    const auto t = *a;
    *a = *b;
    *b = t;
}

// Constexpr version of algorithm::reverse
template <class Iterator>
constexpr void reverse(Iterator first, Iterator last) {
    for (; first != last and first != --last; ++first) {
        iter_swap(first, last);
    }
}

constexpr std::size_t get_num_words(const std::string_view& text) {
    if (text.back() != END_MARKER) {
        throw std::logic_error("The input text has to be terminated by NULL character.");
    }
    std::size_t n = 0;
    for (auto itr = std::begin(text); itr != std::end(text); ++itr) {
        n += static_cast<std::size_t>(*itr == END_MARKER);
    }
    return n;
}

#ifndef CXPRDA_DISABLE_CONSTEXPR
template <std::size_t N>
constexpr auto text_to_words(const std::string_view& text) {
    if (text.back() != END_MARKER) {
        throw std::logic_error("The input text has to be terminated by NULL character.");
    }
    std::array<std::string_view, N> words = {};
    auto b_itr = std::begin(text);
    for (std::size_t i = 0; i < N; i++) {
        auto e_itr = find(b_itr, std::end(text), END_MARKER) + 1;
        words[i] = text.substr(std::distance(std::begin(text), b_itr), std::distance(b_itr, e_itr));
        b_itr = e_itr;
    }
    return words;
}
#else
auto text_to_words(const std::string_view& text) {
    if (text.back() != END_MARKER) {
        throw std::logic_error("The input text has to be terminated by NULL character.");
    }
    std::vector<std::string_view> words(get_num_words(text));
    auto b_itr = std::begin(text);
    for (std::size_t i = 0; i < N; i++) {
        auto e_itr = find(b_itr, std::end(text), END_MARKER) + 1;
        words[i] = text.substr(std::distance(std::begin(text), b_itr), std::distance(b_itr, e_itr));
        b_itr = e_itr;
    }
    return words;
}
#endif

template <class Words>
constexpr std::size_t get_max_length(const Words& words) {
    std::size_t max_length = 0;
    for (const auto& w : words) {
        max_length = std::max(max_length, std::size(w));
    }
    return max_length;
}

}  // namespace utils

namespace details {

template <class Int = int32_t>
struct unit {
    static_assert(std::is_signed_v<Int>, "The integer type has to be signed.");

    using int_type = Int;

    int_type base;
    int_type check;
};

template <class Words, class Units>
class builder {
  public:
    using words_type = Words;
    using units_type = Units;

    using unit_type = typename Units::value_type;
    using int_type = typename unit_type::int_type;
    using char_type = typename words_type::value_type::value_type;

    static_assert(sizeof(char_type) == 1);

  private:
    const words_type& m_words;
    units_type m_units;

    std::size_t m_id = 0;
    static_vector<uint8_t, 256> m_edges;

  public:
    CXPRDA_CONSTEXPR builder(const Words& words) : m_words(words), m_units(256) {
        init();
        arrange(0, std::size(words), 0, 0);
    }
    CXPRDA_CONSTEXPR units_type steal_units() {
        return std::move(m_units);
    }

  private:
    CXPRDA_CONSTEXPR void init() {
        for (int_type i = 1; i < 256; i++) {
            m_units[i].base = -(i + 1);
            m_units[i].check = -(i - 1);
        }
        m_units[255].base = -1;
        m_units[1].check = -255;
        m_units[0].base = -1;  // As unused flag
        m_units[0].check = 1;  // Empty head
    }

    CXPRDA_CONSTEXPR void enlarge() {
        const auto old_size = static_cast<int_type>(std::size(m_units));
        const auto new_size = old_size + 256;

        for (auto i = old_size; i < new_size; i++) {
            m_units.push_back(unit_type{-(i + 1), -(i - 1)});
        }

        if (m_units[0].check == 0) {  // is full?
            m_units[new_size - 1].base = -old_size;
            m_units[old_size].check = -(new_size - 1);
            m_units[0].check = old_size;
        } else {
            const auto emp_head = m_units[0].check;
            const auto emp_tail = -m_units[emp_head].check;
            m_units[emp_tail].base = -old_size;
            m_units[old_size].check = -emp_tail;
            m_units[new_size - 1].base = -emp_head;
            m_units[emp_head].check = -(new_size - 1);
        }
    }

    CXPRDA_CONSTEXPR void fix_child(std::size_t npos, std::size_t cpos) {
        assert(m_units[npos].check >= 0);
        assert(m_units[cpos].check < 0);

        const auto next = -1 * m_units[cpos].base;
        const auto prev = -1 * m_units[cpos].check;
        m_units[prev].base = -1 * next;
        m_units[next].check = -1 * prev;

        // Update empty head
        if (cpos == m_units[0].check) {
            if (next == cpos) {  // No empty units?
                m_units[0].check = 0;
            } else {
                m_units[0].check = next;
            }
        }

        m_units[cpos].check = static_cast<int_type>(npos);
    }

    CXPRDA_CONSTEXPR void arrange(std::size_t bpos, std::size_t epos, std::size_t depth, std::size_t npos) {
        if (std::size(m_words[bpos]) == depth) {
            if (m_words[bpos].back() != END_MARKER) {
                throw std::logic_error("An input word has to be terminated by NULL character.");
            }
            assert(m_units[npos].base < 0);
            m_units[npos].base = m_id++;
            return;
        }

        m_edges.clear();
        {
            auto c1 = static_cast<uint8_t>(m_words[bpos][depth]);
            for (auto i = bpos + 1; i < epos; i++) {
                const auto c2 = static_cast<uint8_t>(m_words[i][depth]);
                if (c1 != c2) {
                    if (c1 > c2) {
                        throw std::logic_error("The input data is not in lexicographical order.");
                    }
                    m_edges.push_back(c1);
                    c1 = c2;
                }
            }
            m_edges.push_back(c1);
        }

        const auto base = xcheck();
        if (std::size(m_units) <= base) {
            enlarge();
        }

        assert(m_units[npos].base < 0);
        m_units[npos].base = static_cast<int_type>(base);

        for (const auto c : m_edges) {
            fix_child(npos, base ^ static_cast<std::size_t>(c));
        }

        auto i = bpos;
        auto c1 = static_cast<uint8_t>(m_words[bpos][depth]);

        for (auto j = bpos + 1; j < epos; j++) {
            const auto c2 = static_cast<uint8_t>(m_words[j][depth]);
            if (c1 != c2) {
                arrange(i, j, depth + 1, base ^ static_cast<std::size_t>(c1));
                i = j;
                c1 = c2;
            }
        }
        arrange(i, epos, depth + 1, base ^ static_cast<std::size_t>(c1));
    }

    CXPRDA_CONSTEXPR std::size_t xcheck() const {
        assert(!std::empty(m_edges));

        const std::size_t emp_head = m_units[0].check;
        if (emp_head == 0) {
            return std::size(m_units) ^ static_cast<std::size_t>(m_edges[0]);
        }

        std::size_t npos = emp_head;
        do {
            const std::size_t base = npos ^ static_cast<std::size_t>(m_edges[0]);
            if (is_target(base)) {
                return base;
            }
            assert(m_units[npos].base < 0);
            npos = -1 * m_units[npos].base;
        } while (npos != emp_head);

        return std::size(m_units) ^ static_cast<std::size_t>(m_edges[0]);
    }

    CXPRDA_CONSTEXPR bool is_target(const std::size_t base) const {
        for (const auto c : m_edges) {
            const auto npos = base ^ static_cast<std::size_t>(c);
            if (m_units[npos].check >= 0) {
                return false;
            }
        }
        return true;
    }
};

template <class Words>
CXPRDA_CONSTEXPR void get_num_nodes(const Words& words, std::size_t& num_nodes,  //
                                    std::size_t bpos, std::size_t epos, std::size_t depth) {
    if (std::size(words[bpos]) == depth) {
        if (words[bpos].back() != END_MARKER) {
            throw std::logic_error("An input word has to be terminated by NULL character.");
        }
        return;
    }

    auto i = bpos;
    auto c1 = static_cast<uint8_t>(words[bpos][depth]);

    for (auto j = bpos + 1; j < epos; j++) {
        const auto c2 = static_cast<uint8_t>(words[j][depth]);
        if (c1 != c2) {
            if (c1 > c2) {
                throw std::logic_error("The input data is not in lexicographical order.");
            }
            get_num_nodes(words, ++num_nodes, i, j, depth + 1);
            i = j;
            c1 = c2;
        }
    }
    get_num_nodes(words, ++num_nodes, i, epos, depth + 1);
}

}  // namespace details

template <class Words>
CXPRDA_CONSTEXPR std::size_t get_capacity(const Words& words) {
    std::size_t num_nodes = 0;
    details::get_num_nodes(words, num_nodes, 0, std::size(words), 0);
    return (static_cast<std::size_t>(num_nodes * RESERVE_FACTOR) + 255) / 256 * 256;
}

#ifndef CXPRDA_DISABLE_CONSTEXPR
template <std::size_t Capacity, class Words>
constexpr auto make(const Words& words) {
    using units_type = static_vector<details::unit<>, Capacity>;
    details::builder<Words, units_type> b(words);
    return b.steal_units();
}
#else
template <class Words>
auto make(const Words& words) {
    using units_type = std::vector<details::unit<>>;
    details::builder<Words, units_type> b(words);
    auto units = b.steal_units();
    units.shrink_to_fit();
    return units;
}
#endif

struct search_result {
    std::size_t id;
    std::size_t npos;  // node position (for decode)
    std::size_t depth;  // node depth (for decode)
};

template <class Word, class Units>
CXPRDA_CONSTEXPR search_result search(const Word& word, const Units& units) {
    if (word.back() == END_MARKER) {
        throw std::logic_error("The query word should not be terminated by NULL character.");
    }

    std::size_t npos = 0, depth = 0;
    for (; depth < std::size(word); depth++) {
        const std::size_t cpos = units[npos].base ^ static_cast<std::size_t>(word[depth]);
        if (units[cpos].check != npos) {
            return {NOT_FOUND, npos, depth};
        }
        npos = cpos;
    }
    const std::size_t cpos = units[npos].base;  // ^END_MARKER
    if (units[cpos].check != npos) {
        return {NOT_FOUND, npos, depth};
    }
    return {static_cast<std::size_t>(units[cpos].base), cpos, depth + 1};
}

#ifndef CXPRDA_DISABLE_CONSTEXPR

template <std::size_t BufSize, class Word, class Units>
constexpr auto common_prefix_search(const Word& word, const Units& units) {
    static_assert(BufSize != 0);

    if (word.back() == END_MARKER) {
        throw std::logic_error("The query word should not be terminated by NULL character.");
    }

    std::size_t npos = 0, depth = 0;
    static_vector<search_result, BufSize> results;

    for (; depth < std::size(word); depth++) {
        const std::size_t tpos = units[npos].base;  // ^END_MARKER
        if (units[tpos].check == npos) {
            results.push_back(search_result{static_cast<std::size_t>(units[tpos].base), tpos, depth + 1});
            if (results.size() == results.capacity()) {
                return results;
            }
        }
        const std::size_t cpos = units[npos].base ^ static_cast<std::size_t>(word[depth]);
        if (units[cpos].check != npos) {
            return results;
        }
        npos = cpos;
    }

    const std::size_t tpos = units[npos].base;  // ^END_MARKER
    if (units[tpos].check == npos) {
        results.push_back(search_result{static_cast<std::size_t>(units[tpos].base), tpos, depth + 1});
    }
    return results;
}

template <std::size_t BufSize, class Units>
constexpr void enumerate(std::size_t npos, std::size_t depth, const Units& units,
                         static_vector<search_result, BufSize>& results) {
    static_assert(BufSize != 0);

    const std::size_t tpos = units[npos].base;  // ^END_MARKER
    if (units[tpos].check == npos) {
        results.push_back(search_result{static_cast<std::size_t>(units[tpos].base), tpos, depth + 1});
        if (results.size() == results.capacity()) {
            return;
        }
    }

    for (std::size_t c = 1; c < 256; c++) {
        const std::size_t cpos = units[npos].base ^ c;
        if (units[cpos].check == npos) {
            enumerate(cpos, depth + 1, units, results);
            if (results.size() == results.capacity()) {
                return;
            }
        }
    }
}

template <std::size_t BufSize, class Units>
constexpr auto enumerate(const Units& units) {
    static_vector<search_result, BufSize> results;
    enumerate(0, 0, units, results);
    return results;
}

template <std::size_t BufSize, class Word, class Units>
constexpr auto predictive_search(const Word& word, const Units& units) {
    static_assert(BufSize != 0);

    if (word.back() == END_MARKER) {
        throw std::logic_error("The query word should not be terminated by NULL character.");
    }

    std::size_t npos = 0, depth = 0;
    static_vector<search_result, BufSize> results;

    for (; depth < std::size(word); depth++) {
        const std::size_t cpos = units[npos].base ^ static_cast<std::size_t>(word[depth]);
        if (units[cpos].check != npos) {
            return results;
        }
        npos = cpos;
    }

    enumerate(npos, depth, units, results);
    return results;
}

template <std::size_t BufSize, class Units>
constexpr auto decode(std::size_t npos, const Units& units) {
    static_assert(BufSize != 0);

    static_vector<char, BufSize> word;
    if (units[npos].check < 0) {
        return word;
    }

    while (npos != 0) {
        const std::size_t ppos = units[npos].check;
        const char c = static_cast<char>(units[ppos].base ^ npos);
        if (c != END_MARKER) {
            word.push_back(c);
        }
        npos = ppos;
    }

    utils::reverse(word.begin(), word.end());
    return word;
}

#else

template <class Word, class Units>
auto common_prefix_search(const Word& word, const Units& units) {
    if (word.back() == END_MARKER) {
        throw std::logic_error("The query word should not be terminated by NULL character.");
    }

    std::size_t npos = 0, depth = 0;
    std::vector<search_result> results;

    for (; depth < std::size(word); depth++) {
        const std::size_t tpos = units[npos].base;  // ^END_MARKER
        if (units[tpos].check == npos) {
            results.push_back(search_result{static_cast<std::size_t>(units[tpos].base), tpos, depth + 1});
        }
        const std::size_t cpos = units[npos].base ^ static_cast<std::size_t>(word[depth]);
        if (units[cpos].check != npos) {
            return results;
        }
        npos = cpos;
    }

    const std::size_t tpos = units[npos].base;  // ^END_MARKER
    if (units[tpos].check == npos) {
        results.push_back(search_result{static_cast<std::size_t>(units[tpos].base), tpos, depth + 1});
    }
    return results;
}

template <class Units>
void enumerate(std::size_t npos, std::size_t depth, const Units& units, std::vector<search_result>& results) {
    const std::size_t tpos = units[npos].base;  // ^END_MARKER
    if (units[tpos].check == npos) {
        results.push_back(search_result{static_cast<std::size_t>(units[tpos].base), tpos, depth + 1});
    }

    for (std::size_t c = 1; c < 256; c++) {
        const std::size_t cpos = units[npos].base ^ c;
        if (units[cpos].check == npos) {
            enumerate(cpos, depth + 1, units, results);
        }
    }
}

template <class Units>
auto enumerate(const Units& units) {
    std::vector<search_result> results;
    enumerate(0, 0, units, results);
    return results;
}

template <class Word, class Units>
auto predictive_search(const Word& word, const Units& units) {
    if (word.back() == END_MARKER) {
        throw std::logic_error("The query word should not be terminated by NULL character.");
    }

    std::size_t npos = 0, depth = 0;
    std::vector<search_result> results;

    for (; depth < std::size(word); depth++) {
        const std::size_t cpos = units[npos].base ^ static_cast<std::size_t>(word[depth]);
        if (units[cpos].check != npos) {
            return results;
        }
        npos = cpos;
    }

    enumerate(npos, depth, units, results);
    return results;
}

template <class Units>
auto decode(std::size_t npos, const Units& units) {
    std::vector<char> word;
    if (units[npos].check < 0) {
        return word;
    }

    while (npos != 0) {
        const std::size_t ppos = units[npos].check;
        const char c = static_cast<char>(units[ppos].base ^ npos);
        if (c != END_MARKER) {
            word.push_back(c);
        }
        npos = ppos;
    }
    utils::reverse(word.begin(), word.end());
    return word;
}

#endif

}  // namespace constexpr_doublearray
