# constexpr_doublearray

## What is this?

This library implements a double-array trie of **compile-time constants** using `constexpr` in C++17.

As you know, compile-time computation is awesome. Also, the double array is awesome. Hence, this library is awesome (QED).

## Useful?

I don't know.

## Installation 

Please through the path to the directory `include`.

```shell
$ ls -1 include
constexpr_doublearray.hpp
fixed_capacity_vector
```

- `constexpr_doublearray.hpp`: The `constexpr` double-array implementation (and some utils).
- `fixed_capacity_vector`: [gnzlbg's static_vector](https://github.com/gnzlbg/static_vector) that is a `constexpr` version of `std::vector` with fixed capacity.

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
#include <constexpr_doublearray.hpp>

using namespace std::string_view_literals;

// Input words concatenated by NULL character (must be sorted)
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

*Note:* The double array can contain some vacant elements, and the actual number (i.e., the array capacity) is not known until it is constructed. To avoid dynamic sizing at compile time, `get_capacity` estimates the capacity from input words. So, depending on the estimate, the construction may fail. If this happens, please increase the value of `RESERVE_FACTOR`. However, the larger the value, the larger the memory consumption will be.

If you know of a more elegant way to do this, could you please let me know?

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
    std::cout << "decode(icde_sr) = " << std::string(std::begin(icde_dec), std::end(icde_dec)) << std::endl;
    std::cout << "decode(sigmod_sr) = " << std::string(std::begin(sigmod_dec), std::end(sigmod_dec)) << std::endl;
}
```

The output will be

```
decode(icde_sr) = ICDE
decode(sigmod_sr) = SIGMOD
```

### Common prefix search

```c++
// Common prefix search
constexpr auto icdmw_size = std::size("ICDMW"sv);
constexpr auto icdmw_cpsr = constexpr_doublearray::common_prefix_search<icdmw_size>("ICDMW"sv, dict);

int main() {
    std::cout << "common_prefix_search(ICDMW) = ";
    for (auto r : icdmw_cpsr) {
        const auto dec = constexpr_doublearray::decode<icdmw_size>(r.npos, dict);
        std::cout << "(id=" << r.id << ",str=" << std::string(std::begin(dec), std::end(dec)) << "), ";
    }
    std::cout << std::endl;
}
```

The output will be

```
common_prefix_search(ICDMW) = (id=1,str=ICDM), (id=2,str=ICDMW), 
```

### Predictive search

```c++
// Predictive search
constexpr auto sig_psr = constexpr_doublearray::predictive_search<num_words>("SIG"sv, dict);
// Buffer size for decoding
constexpr auto decode_size = constexpr_doublearray::utils::get_max_length(words);

int main() {
    std::cout << "predictive_search(SIG) = ";
    for (auto r : sig_psr) {
        const auto dec = constexpr_doublearray::decode<decode_size>(r.npos, dict);
        std::cout << "(id=" << r.id << ",str=" << std::string(std::begin(dec), std::end(dec)) << "), ";
    }
    std::cout << std::endl;
}
```

The output will be

```
predictive_search(SIG) = (id=4,str=SIGIR), (id=5,str=SIGMOD), 
```

## Demo

`demo.cpp` provides a command line tool, which shows query results for the dataset defined in  `dataset.hpp`.

```shell
$ ./demo
Query modes:
 - 1: search
 - 2: common_prefix_search
 - 3: predictive_search
 - others: exit
Input query mode (int):
> 1
Input query word (string):
> university_of_tokushima
id = 745
Input query mode (int):
> 2                
Input query word (string):
> kyushu_university_of_health_and_welfare
1: id = 329, str = kyushu_university
2: id = 330, str = kyushu_university_of_health_and_welfare
Input query mode (int):
> 3
Input query word (string):
> japan
1: id = 180, str = japan_advanced_institute_of_science_and_technology
2: id = 181, str = japan_christian_junior_college
3: id = 182, str = japan_lutheran_college
4: id = 183, str = japan_red_cross_aichi_junior_college_of_nursing
5: id = 184, str = japanese_red_cross_college_of_nursing
Input query mode (int):
> 0
Good bye!!
```

Through Python script `txt2hpp.py`, you can produce `dataset.hpp` from a text file with line-separated words.

```shell
$ head -5 univs.txt
aichi_bunkyo_university
aichi_gakuin_university
aichi_institute_of_technology
aichi_kiwami_college_of_nursing
aichi_konan_college
$ python txt2hpp.py univs.txt
```

## Relaxation of `constexpr` restrictions 

Most compilers restrict the number of evaluated expressions at compile time. For example, in [Clang](https://clang.llvm.org/docs/UsersManual.html), the limits are indicated by

- `-fconstexpr-depth`: The limit for recursive `constexpr` function calls. The default value is 512.
- `-fconstexpr-steps`: The limit for the number of full-expressions evaluated in a single constant expression evaluation. The default value is 1048576.

When constructing the double array from a large dataset, the construction may fail with the default values. In `make` and `predictive_search`, the trie is traversed recursively, and the maximum depth is the maximum length of the stored words. That is, you need to set `-fconstexpr-depth` to the maximum length at least. Also, in `make`, there are some parts that loop through the array, `-fconstexpr-steps` should be set to a sufficiently large value.

In Clang, setting `-fconstexpr-depth=-1 -fconstexpr-steps=-1` provides the largest values. This is a hack-like approach and may fail depending on Clang updates. At least, in AppleClang 12.0.0, it works.

In [GNU](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html), `-fconstexpr-loop-limit` is used instead of `-fconstexpr-steps`. Note that such a negative parameter will be invalid.
