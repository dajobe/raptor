#!/bin/sh
# Helper script to install bison 3 (primarily for travis CI)

set -x

PACKAGE=bison
MIN_VERSION=3.0.0
INSTALL_VERSION=3.0.2

# Bison requires that CC is a C compiler
CC=cc
export CC

AWK=${AWK:-awk}
BISON=${BISON:-bison}
CURL=${CURL:-curl}
WGET=${WGET:-wget}

FILE="$PACKAGE-$INSTALL_VERSION.tar.gz"
URL="http://ftp.gnu.org/gnu/bison/$PACKAGE-$INSTALL_VERSION.tar.gz"

ROOT_DIR=${TMPDIR:-/tmp}
BUILD_DIR="$ROOT_DIR/build-$PACKAGE"

installed_version=`$BISON --version 2>&1 | sed -ne 's/^.*GNU Bison[^0-9]*//p'`
installed_version_dec=`echo $installed_version | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
min_version_dec=`echo $MIN_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
if test $installed_version_dec -ge $min_version_dec; then
    echo "$PACKAGE $installed_version is new enough"
else
    mkdir $BUILD_DIR && cd $BUILD_DIR
    $WGET -O $FILE $URL || $CURL -o $FILE $URL
    tar -x -z -f $FILE && rm $FILE
    cd $PACKAGE-$INSTALL_VERSION && ./configure --prefix=/usr && make && sudo make install
    cd / && rm -rf $BUILD_DIR
fi
