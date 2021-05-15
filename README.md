# constexpr_doublearray

## What is this?

This library implements a double-array trie of **compile-time constants** using `constexpr` of C++17.

As you know, compile-time computation is awesome. Also, the double array is awesome. Hence, this library is awesome (QED).

## Useful?

I don't know.

## Installation 

Add the header files `constexpr_doublearray.hpp` and `fixed_capacity_vector`  to your own project. The latter is [gnzlbg's static_vector](https://github.com/gnzlbg/static_vector), a `constexpr std::vector` implementation with fixed capacity.

## Build instructions

You can download and compile the library with the following commands:

```shell
$ git clone https://github.com/kampersanda/constexpr_doublearray.git
$ cd constexpr_doublearray
$ mkdir build
$ cd build
$ cmake ..
$ make -j
```

The code is written in C++17, so please install g++ >= 7.0 or clang >= 4.0. For the build system, CMake >= 3.0 have to be installed to compile the library.

Currently, the code has been tested only on Mac OS X with AppleClang 12.0.0.

## Sample

 `sample.cpp` provides a sample code, which works as follows.

### Preparation

```c++
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
```

### Construction

```c++
// Double-array dictionary
constexpr auto capacity = constexpr_doublearray::get_capacity(words);
constexpr auto dict = constexpr_doublearray::make<capacity>(words);
```

Here, `capacity` is

### Simple search

```c++
// Exact search
constexpr auto icde_sr = constexpr_doublearray::search("ICDE"sv, dict);
constexpr auto sigmod_sr = constexpr_doublearray::search("SIGMOD"sv, dict);
constexpr auto sigkdd_sr = constexpr_doublearray::search("SIGKDD"sv, dict);

int main() {
    std::cout << "search(ICDE) = " << icde_sr.id << std::endl;
    std::cout << "search(SIGMOD) = " << sigmod_sr.id << std::endl;
    std::cout << "search(SIGKDD) = " << sigkdd_sr.id << std::endl;
}
```

The output will be

```
search(ICDE) = 0
search(SIGMOD) = 5
search(SIGKDD) = 18446744073709551615
```

### Decoding

```c++
// Decoding
constexpr auto icde_dec = constexpr_doublearray::decode<icde_sr.depth>(icde_sr.npos, dict);
constexpr auto sigmod_dec = constexpr_doublearray::decode<sigmod_sr.depth>(sigmod_sr.npos, dict);

int main() {
    std::cout << "decode(icde_dec) = ";
    std::cout << std::string(std::begin(icde_dec), std::end(icde_dec)) << std::endl;
    std::cout << "decode(sigmod_dec) = ";
    std::cout << std::string(std::begin(sigmod_dec), std::end(sigmod_dec)) << std::endl;
}
```

The output will be

```
decode(icde_dec) = ICDE
decode(sigmod_dec) = SIGMOD
```

### Common prefix search

```c++
// Common prefix search
constexpr auto icdmw_cpsr = constexpr_doublearray::common_prefix_search<5>("ICDMW"sv, dict);

    std::cout << "common_prefix_search(ICDMW) =" << std::endl;
    for (auto r : icdmw_cpsr) {
        const auto dec = constexpr_doublearray::decode<std::size("ICDMW"sv)>(r.npos, dict);
        std::cout << "\t(id=" << r.id << ", str=" << std::string(std::begin(dec), std::end(dec)) << ")," << std::endl;
    }
```

The output will be

```
common_prefix_search(ICDMW) =
	(id=1, str=ICDM),
	(id=2, str=ICDMW),
```

### Predictive search

```c++
// Predictive search
constexpr auto sig_psr = constexpr_doublearray::predictive_search<2>("SIG"sv, dict);
// Buffer size for decoding
constexpr auto decode_size = constexpr_doublearray::utils::get_max_length(words);

int main() {
    std::cout << "predictive_search(SIG) =" << std::endl;
    for (auto r : sig_psr) {
        const auto dec = constexpr_doublearray::decode<decode_size>(r.npos, dict);
        std::cout << "\t(id=" << r.id << ", str=" << std::string(std::begin(dec), std::end(dec)) << ")," << std::endl;
    }
}
```

The output will be

```
predictive_search(SIG) =
	(id=4, str=SIGIR),
	(id=5, str=SIGMOD),
```

## Demo

