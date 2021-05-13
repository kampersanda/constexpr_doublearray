#pragma once

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string_view>

namespace constexpr_doublearray {

static constexpr double RESERVE_FACTOR = 1.1;

// Pair type of (Base, Check)
template <class Int>
class unit {
    static_assert(std::is_signed_v<Int>, "The integer type has to be signed.");

  public:
    using int_type = Int;

  private:
    int_type m_base;
    int_type m_check;

  public:
    constexpr unit(int_type base = -1, int_type check = -1) : m_base(base), m_check(check) {}

    constexpr int_type get_base() const {
        return m_base;
    }
    constexpr int_type get_check() const {
        return m_check;
    }
    constexpr void set_base(int_type base) {
        m_base = base;
    }
    constexpr void set_check(int_type check) {
        m_check = check;
    }
};

namespace details {

// Constexpr version of algorithm::find
template <class Iterator, class Value>
constexpr Iterator find(Iterator first, Iterator last, const Value& value) {
    for (; first != last; ++first) {
        if (*first == value) break;
    }
    return first;
}

template <class Words>
constexpr void get_num_nodes(const Words& words, std::size_t& num_nodes,  //
                             std::size_t bpos, std::size_t epos, std::size_t depth) {
    if (std::size(words[bpos]) == depth) {
        return;
    }

    auto i = bpos;
    auto c1 = words[bpos][depth];

    for (auto j = bpos + 1; j < epos; j++) {
        const auto c2 = words[j][depth];
        if (c1 != c2) {
            get_num_nodes(words, ++num_nodes, i, j, depth + 1);
            i = j;
            c1 = c2;
        }
    }
    get_num_nodes(words, ++num_nodes, i, epos, depth + 1);
}

template <class Words, class Units>
class builder {
  public:
    using int_type = typename Units::value_type::int_type;
    using char_type = typename Words::value_type::value_type;

  private:
    const Words& m_words;
    Units m_units;

    int_type m_id = 0;

    std::array<char_type, 256> m_edges = {};
    std::size_t m_num_edges = 0;

  public:
    constexpr builder(const Words& words) : m_words(words) {
        arrange(0, std::size(words), 0, 0);
    }
    constexpr Units steal_units() {
        return std::move(m_units);
    }

  private:
    constexpr void arrange(std::size_t bpos, std::size_t epos, std::size_t depth, std::size_t npos) {
        if (std::size(m_words[bpos]) == depth) {
            m_units[npos].set_base(m_id++);
            return;
        }

        m_num_edges = 0;
        {
            auto c1 = m_words[bpos][depth];
            for (auto i = bpos + 1; i < epos; i++) {
                const auto c2 = m_words[i][depth];
                if (c1 != c2) {
                    m_edges[m_num_edges++] = c1;
                    c1 = c2;
                }
            }
            m_edges[m_num_edges++] = c1;
        }

        const auto base = xcheck();
        if (base == std::numeric_limits<std::size_t>::max()) {
            throw std::logic_error("The capacity is not enough.");
        }

        m_units[npos].set_base(static_cast<int_type>(base));

        for (std::size_t i = 0; i < m_num_edges; i++) {
            const auto cpos = base ^ static_cast<std::size_t>(m_edges[i]);
            m_units[cpos].set_check(static_cast<int_type>(npos));
        }

        auto i = bpos;
        auto c1 = m_words[bpos][depth];

        for (auto j = bpos + 1; j < epos; j++) {
            const auto c2 = m_words[j][depth];
            if (c1 != c2) {
                arrange(i, j, depth + 1, base ^ static_cast<std::size_t>(c1));
                i = j;
                c1 = c2;
            }
        }
        arrange(i, epos, depth + 1, base ^ static_cast<std::size_t>(c1));
    }

    constexpr std::size_t xcheck() {
        for (std::size_t base = 0; base < m_units.size(); base++) {
            if (is_target(base)) {
                return base;
            }
        }
        return std::numeric_limits<std::size_t>::max();
    }

    constexpr bool is_target(const std::size_t base) {
        for (std::size_t i = 0; i < m_num_edges; i++) {
            const auto npos = base ^ std::size_t(m_edges[i]);
            if (m_units[npos].get_check() >= 0) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace details

namespace utils {

constexpr std::size_t get_num_words(const std::string_view& text) {
    if (text.back() != '\0') {
        throw std::logic_error("The input text has to be terminated by NULL character.");
    }
    std::size_t n = 0;
    for (auto itr = std::begin(text); itr != std::end(text); ++itr) {
        n += static_cast<std::size_t>(*itr == '\0');
    }
    return n;
}

template <std::size_t N>
constexpr auto text_to_words(const std::string_view& text) {
    if (text.back() != '\0') {
        throw std::logic_error("The input text has to be terminated by NULL character.");
    }
    std::array<std::string_view, N> words = {};
    auto b_itr = std::begin(text);
    for (std::size_t i = 0; i < N; i++) {
        auto e_itr = details::find(b_itr, std::end(text), '\0') + 1;
        words[i] = text.substr(std::distance(std::begin(text), b_itr), std::distance(b_itr, e_itr));
        b_itr = e_itr;
    }
    return words;
}

}  // namespace utils

template <class Words>
constexpr std::size_t get_capacity(const Words& words) {
    std::size_t num_nodes = 0;
    details::get_num_nodes(words, num_nodes, 0, std::size(words), 0);
    return (static_cast<std::size_t>(num_nodes * RESERVE_FACTOR) + 255) / 256 * 256;
}

template <std::size_t N, class Words, class Unit = unit<int>>
constexpr auto make(const Words& words) {
    details::builder<Words, std::array<Unit, N>> b(words);
    return b.steal_units();
}

template <class Units, class Word>
constexpr std::size_t search(const Units& units, const Word& word) {
    std::size_t npos = 0;
    for (const auto c : word) {
        const std::size_t cpos = units[npos].get_base() ^ static_cast<std::size_t>(c);
        if (units[cpos].get_check() != npos) {
            return std::numeric_limits<std::size_t>::max();
        }
        npos = cpos;
    }

    const std::size_t cpos = units[npos].get_base();  // ^ '\0'
    if (units[cpos].get_check() != npos) {
        return std::numeric_limits<std::size_t>::max();
    } else {
        return static_cast<std::size_t>(units[npos].get_base());
    }
}

}  // namespace constexpr_doublearray
