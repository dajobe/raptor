#!/usr/bin/env python3
#
# Edit main header so that gtk-doc is happy about it
#
# USAGE:
#   python3 fix-gtkdoc-header.py < header.h > header.i
#
# Copyright (C) 2025, David Beckett http://www.dajobe.org/
#
# This package is Free Software and part of Redland http://librdf.org/
#
# It is licensed under the following three licenses as alternatives:
#   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
#
# You may not use this file except in compliance with at least one of
# the above three licenses.
#
# See LICENSE.html or LICENSE.txt at the top of this package for the
# complete terms and further detail along with the license texts for
# the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.

import sys
import re


def main():
    """
    Main function to process C header from stdin and write to stdout.
    """
    for line in sys.stdin:
        # Remove trailing macros
        line = re.sub(r"(RASQAL|RAPTOR)_PRINTF_FORMAT\(\d+, \d+\);", ";", line)

        # gtk-doc hates const in some places
        line = re.sub(r"const char\* const\* (\w+)", r"const char* \1", line)

        # and unsigned char
        line = re.sub(r"const unsigned char \*(\w+)", r"const char * \1", line)

        # and unsigned char in a handler typedef
        line = re.sub(r"typedef unsigned char\s*\*", "typedef char *", line)

        print(line, end="")


if __name__ == "__main__":
    main()
