#!/bin/sh
# Helper script to install bison 3 into system via SUDO
#
# This is primarily for travis CI

set -x


# Binary that is installed that should be checked for version
program_name=${BISON:-bison}
# Name of package - will be tarball prefix
package_name=${PACKAGE:-bison}


########### Should be generic for any GNU package below here  ########

AWK=${AWK:-awk}
CURL=${CURL:-curl}
WGET=${WGET:-wget}

package_ucname=$(echo "${package_name}" | tr '[:lower:]' '[:upper:]')
package_min_version=$($AWK -F= "/^${package_ucname}_MIN_VERSION/ {print \$2}" configure.ac)
package_rec_version=$($AWK -F= "/^${package_ucname}_REC_VERSION/ {print \$2}" configure.ac)


# Package specific overrides
case "$package_name" in
  bison)
    # Bison requires that CC is a C compiler
    CC=cc
    export CC
    ;;
  *)
    ;;
esac

DESTDIR=${DESTDIR:-/usr}

ROOT_DIR=${TMPDIR:-/tmp}
BUILD_DIR="$ROOT_DIR/build-${package_name}"

installed_version=$(${program_name} --version 2>&1 | $AWK "/^${package_name}.*[.0-9]\$/ { print \$NF}")
# shellcheck disable=SC2016
installed_version_dec=$(echo "$installed_version" | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};')
# shellcheck disable=SC2016
min_version_dec=$(echo "$package_min_version" | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};')

if test "$installed_version_dec" -ge "$min_version_dec"; then
    echo "${package_name} $installed_version is new enough"
else
    echo "Building and installing ${package_rec_version} from source into ${DESTDIR}"
    mkdir "$BUILD_DIR" && cd "$BUILD_DIR" || exit 1
    package_dir="${package_name}-${package_rec_version}"
    tarball_file="${package_dir}.tar.gz"
    package_url="http://ftp.gnu.org/gnu/${package_name}/${tarball_file}"

    $WGET -O "$tarball_file" "$package_url" || \
      $CURL -o "$tarball_file" "$package_url"
    tar -x -z -f "$tarball_file" && rm "$tarball_file"
    cd "${package_dir}" && \
        ./configure "--prefix=${DESTDIR}" && \
        make && \
        sudo make install
    cd / && rm -rf "$BUILD_DIR"
fi
