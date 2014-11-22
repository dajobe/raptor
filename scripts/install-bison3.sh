#!/bin/sh
# Helper script to install newer bison 3 for travis CI

set -x

V=3.0.2

# Bison requires CC is a C compiler
CC=cc
export CC

cd /tmp
wget http://ftp.gnu.org/gnu/bison/bison-$V.tar.gz
tar -x -z -f bison-$V.tar.gz
cd bison-$V && ./configure --prefix=/usr && make && sudo make install
cd ..
rm -rf bison-$V
