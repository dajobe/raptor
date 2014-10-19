#!/bin/sh
# Helper script to install newer bison 3 for travis CI

set -x

V=3.0.2

wget http://ftp.gnu.org/gnu/bison/bison-$V.tar.gz
tar -x -z -f bison-$V.tar.gz
cd bison-$V && ./configure --prefix=/usr && make && make install
