#!/usr/bin/perl -w
#
# Edit raptor.h so that gtk-doc is happy about it
#
# USAGE:
#   perl fix-gtkc-header.pl < raptor.h > raptor.i
#
# Copyright (C) 2010, David Beckett http://www.dajobe.org/
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

while(<>) {
  # Remove trailing macros
  s{RAPTOR_PRINTF_FORMAT\(\d+, \d+\);}{;};

  # gtk-doc hates const in some places
  s/const char\* const\* (\w+)/const char\* $1/;
  print;
}
