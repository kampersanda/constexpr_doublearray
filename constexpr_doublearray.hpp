#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "fixed_capacity_vector"

namespace constexpr_doublearray {

template <class T, std::size_t Capacity>
using static_vector = std::experimental::fixed_capacity_vector<T, Capacity>;

static constexpr double RESERVE_FACTOR = 1.1;
static constexpr char END_MARKER = '\000';
static constexpr auto NOT_FOUND = std::numeric_limits<std::size_t>::max();

struct unit_type {
    using int_type = int32_t;

    int_type base;
    int_type check;
};

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

template <std::size_t Capacity, class Words>
class builder {
  public:
    using int_type = typename unit_type::int_type;
    using char_type = typename Words::value_type::value_type;

    static_assert(sizeof(char_type) == 1);

  private:
    const Words& m_words;
    static_vector<unit_type, Capacity> m_units;

    std::size_t m_id = 0;
    std::size_t m_size = 1;

    std::array<uint8_t, 256> m_edges = {};
    std::size_t m_num_edges = 0;

  public:
    constexpr builder(const Words& words) : m_words(words), m_units(Capacity) {
        init();
        arrange(0, std::size(words), 0, 0);
        m_units.resize((m_size + 255) / 256 * 256);
    }
    constexpr auto steal_units() {
        return std::move(m_units);
    }

  private:
    constexpr void init() {
        const auto size = static_cast<int_type>(std::size(m_units));
        for (int_type i = 1; i < size; i++) {
            m_units[i].base = -1 * (i + 1);
            m_units[i].check = -1 * (i - 1);
        }
        m_units[size - 1].base = -1;
        m_units[1].check = -1 * (size - 1);
        m_units[0].base = -1;  // unused flag
        m_units[0].check = 1;  // Empty head
    }

    constexpr void fix_child(std::size_t npos, std::size_t cpos) {
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
        m_size = std::max(m_size, cpos + 1);
    }

    constexpr void arrange(std::size_t bpos, std::size_t epos, std::size_t depth, std::size_t npos) {
        if (std::size(m_words[bpos]) == depth) {
            if (m_words[bpos].back() != END_MARKER) {
                throw std::logic_error("An input word has to be terminated by NULL character.");
            }
            assert(m_units[npos].base < 0);
            m_units[npos].base = m_id++;
            return;
        }

        m_num_edges = 0;
        {
            auto c1 = static_cast<uint8_t>(m_words[bpos][depth]);
            for (auto i = bpos + 1; i < epos; i++) {
                const auto c2 = static_cast<uint8_t>(m_words[i][depth]);
                if (c1 != c2) {
                    if (c1 > c2) {
                        throw std::logic_error("The input data is not in lexicographical order.");
                    }
                    m_edges[m_num_edges++] = c1;
                    c1 = c2;
                }
            }
            m_edges[m_num_edges++] = c1;
        }

        const auto base = xcheck();
        if (base == NOT_FOUND) {
            throw std::logic_error("The capacity is not enough. Increase RESERVE_FACTOR.");
        }

        assert(m_units[npos].base < 0);
        m_units[npos].base = static_cast<int_type>(base);

        for (std::size_t i = 0; i < m_num_edges; i++) {
            fix_child(npos, base ^ static_cast<std::size_t>(m_edges[i]));
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

    constexpr std::size_t xcheck() {
        assert(m_num_edges != 0);

        const std::size_t epos = m_units[0].check;
        if (epos == 0) {
            return NOT_FOUND;
        }

        std::size_t npos = epos;
        do {
            const std::size_t base = npos ^ std::size_t(m_edges[0]);
            if (is_target(base)) {
                return base;
            }
            assert(m_units[npos].base < 0);
            npos = -1 * m_units[npos].base;
        } while (npos != epos);

        return NOT_FOUND;
    }

    constexpr bool is_target(const std::size_t base) {
        for (std::size_t i = 0; i < m_num_edges; i++) {
            const auto npos = base ^ std::size_t(m_edges[i]);
            if (m_units[npos].check >= 0) {
                return false;
            }
        }
        return true;
    }
};

template <class Words>
constexpr void get_num_nodes(const Words& words, std::size_t& num_nodes,  //
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
constexpr std::size_t get_capacity(const Words& words) {
    std::size_t num_nodes = 0;
    details::get_num_nodes(words, num_nodes, 0, std::size(words), 0);
    return (static_cast<std::size_t>(num_nodes * RESERVE_FACTOR) + 255) / 256 * 256;
}

template <std::size_t Capacity, class Words>
constexpr auto make(const Words& words) {
    details::builder<Capacity, Words> b(words);
    return b.steal_units();  // the move may not make sense
}

struct search_result {
    std::size_t id;
    std::size_t npos;
    std::size_t depth;
};

template <class Word, class Units>
constexpr search_result search(const Word& word, const Units& units) {
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
        word.push_back(static_cast<char>(units[ppos].base ^ npos));
        npos = ppos;
    }
    utils::reverse(word.begin(), word.end());
    return word;
}

}  // namespace constexpr_doublearray
