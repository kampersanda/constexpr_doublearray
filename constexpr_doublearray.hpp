#pragma once

#include <algorithm>
#include <array>
#include <cassert>
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

namespace utils {

// Constexpr version of algorithm::find
template <class Iterator, class Value>
constexpr Iterator find(Iterator first, Iterator last, const Value& value) {
    for (; first != last; ++first) {
        if (*first == value) break;
    }
    return first;
}

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
        auto e_itr = find(b_itr, std::end(text), '\0') + 1;
        words[i] = text.substr(std::distance(std::begin(text), b_itr), std::distance(b_itr, e_itr));
        b_itr = e_itr;
    }
    return words;
}

}  // namespace utils

namespace details {

template <class Words, class Units>
class builder {
  public:
    using int_type = typename Units::value_type::int_type;
    using char_type = typename Words::value_type::value_type;

    Units m_units;

  private:
    const Words& m_words;

    std::size_t m_id = 0;
    std::size_t m_size = 1;

    std::array<char_type, 256> m_edges = {};
    std::size_t m_num_edges = 0;

  public:
    constexpr builder(const Words& words) : m_words(words) {
        init();
        arrange(0, std::size(words), 0, 0);
        m_units[0].set_check((m_size + 255) / 256 * 256);  // set the occupied size
    }

  private:
    constexpr void init() {
        const auto size = static_cast<int_type>(std::size(m_units));
        for (int_type i = 1; i < size; i++) {
            m_units[i].set_base(-1 * (i + 1));
            m_units[i].set_check(-1 * (i - 1));
        }
        m_units[size - 1].set_base(-1);
        m_units[1].set_check(-1 * (size - 1));
        m_units[0].set_check(1);  // Empty head
    }

    constexpr void fix_child(std::size_t npos, std::size_t cpos) {
        assert(m_units[npos].get_check() >= 0);
        assert(m_units[cpos].get_check() < 0);

        const auto next = -1 * m_units[cpos].get_base();
        const auto prev = -1 * m_units[cpos].get_check();
        m_units[prev].set_base(-1 * next);
        m_units[next].set_check(-1 * prev);

        // Update empty head
        if (cpos == m_units[0].get_check()) {
            if (next == cpos) {  // No empty units?
                m_units[0].set_check(0);
            } else {
                m_units[0].set_check(next);
            }
        }

        m_units[cpos].set_check(static_cast<int_type>(npos));
        m_size = std::max(m_size, cpos + 1);
    }

    constexpr void arrange(std::size_t bpos, std::size_t epos, std::size_t depth, std::size_t npos) {
        if (std::size(m_words[bpos]) == depth) {
            if (m_words[bpos].back() != '\0') {
                throw std::logic_error("An input word has to be terminated by NULL character.");
            }
            assert(m_units[npos].get_base() < 0);
            m_units[npos].set_base(m_id++);
            return;
        }

        m_num_edges = 0;
        {
            auto c1 = m_words[bpos][depth];
            for (auto i = bpos + 1; i < epos; i++) {
                const auto c2 = m_words[i][depth];
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
        if (base == std::numeric_limits<std::size_t>::max()) {
            throw std::logic_error("The capacity is not enough. Increase RESERVE_FACTOR.");
        }

        assert(m_units[npos].get_base() < 0);
        m_units[npos].set_base(static_cast<int_type>(base));

        for (std::size_t i = 0; i < m_num_edges; i++) {
            fix_child(npos, base ^ static_cast<std::size_t>(m_edges[i]));
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
        assert(m_num_edges != 0);

        const std::size_t epos = m_units[0].get_check();
        if (epos == 0) {
            return std::numeric_limits<std::size_t>::max();
        }

        std::size_t npos = epos;
        do {
            const std::size_t base = npos ^ std::size_t(m_edges[0]);
            if (is_target(base)) {
                return base;
            }
            assert(m_units[npos].get_base() < 0);
            npos = -1 * m_units[npos].get_base();
        } while (npos != epos);

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

template <class Words>
constexpr void get_num_nodes(const Words& words, std::size_t& num_nodes,  //
                             std::size_t bpos, std::size_t epos, std::size_t depth) {
    if (std::size(words[bpos]) == depth) {
        if (words[bpos].back() != '\0') {
            throw std::logic_error("An input word has to be terminated by NULL character.");
        }
        return;
    }

    auto i = bpos;
    auto c1 = words[bpos][depth];

    for (auto j = bpos + 1; j < epos; j++) {
        const auto c2 = words[j][depth];
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

template <class Units>
constexpr std::size_t get_size(const Units& units) {
    return units[0].get_check();
}

template <std::size_t Capacity, class Words, class Unit = unit<int>>
constexpr std::array<Unit, Capacity> make(const Words& words) {
    details::builder<Words, std::array<Unit, Capacity>> b(words);
    return std::move(b.m_units);  // maybe, the move does not make sense
}

template <std::size_t Size, std::size_t Capacity, class Unit = unit<int>>
constexpr std::array<Unit, Size> shrink_to_fit(const std::array<Unit, Capacity>& units) {
    std::array<Unit, Size> new_units = {};
    for (std::size_t i = 0; i < Size; i++) {
        new_units[i] = units[i];
    }
    return new_units;
}

template <class Units, class Word>
constexpr std::tuple<std::size_t, std::size_t> search(const Units& units, const Word& word) {
    std::size_t npos = 0;
    for (const auto c : word) {
        const std::size_t cpos = units[npos].get_base() ^ static_cast<std::size_t>(c);
        if (units[cpos].get_check() != npos) {
            return {npos, std::numeric_limits<std::size_t>::max()};
        }
        npos = cpos;
    }

    const std::size_t cpos = units[npos].get_base();  // ^'\0'
    if (units[cpos].get_check() != npos) {
        return {npos, std::numeric_limits<std::size_t>::max()};
    } else {
        return {cpos, static_cast<std::size_t>(units[cpos].get_base())};
    }
}

}  // namespace constexpr_doublearray
