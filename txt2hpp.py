#!/usr/bin/env python3

import sys

# https://stackoverflow.com/questions/45612822/how-to-properly-add-hex-escapes-into-a-string-literal


def main():
    txt_fn = sys.argv[1]
    hpp_fn = 'dataset.hpp'

    fout = open(hpp_fn, 'wb')
    fout.write(b'#pragma once\n')
    fout.write(b'#include <string_view>\n')
    fout.write(b'using namespace std::string_view_literals;\n')
    fout.write(b'static constexpr auto text = ')
    for w in open(txt_fn, 'rb'):
        fout.write(b'\n\"')
        fout.write(w.rstrip())
        fout.write(b'\\0\"')
    fout.write(b'sv;\n')
    fout.close()


if __name__ == "__main__":
    main()
