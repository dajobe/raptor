#!/bin/sh
#
# autogen.sh - Generates the initial makefiles from a pristine CVS tree
#
# $Id$
#
# USAGE: autogen.sh [configure options]
#
# If environment variable DRYRUN is set, no configuring will be done -
# (e.g. in bash)  DRYRUN=1 ./autogen.sh
# will not do any configuring but will emit the programs that would be run.
#
# This script is based on similar scripts used in various tools
# commonly made available via CVS and used with GNU automake.
# Try 'locate autogen.sh' on your system and see what you get.
#
# This script is in the public domain
#

PACKAGE=raptor

SRCDIR=.

# Where the GNU config.sub, config.guess might be found
CONFIG_DIR=../config


DIE=

if test "X$DRYRUN" != X; then
  DRYRUN=echo
fi

# automake 1.8 requires autoconf 2.58
# automake 1.7 requires autoconf 2.54
automake_min_vers=1.7
aclocal_min_vers=$automake_min_vers
autoconf_min_vers=2.54
libtoolize_min_vers=1.4

automake_args=--add-missing
autoconf_args=
aclocal_args=


program=`basename $0`

# START Find autoconf and automake
save_args=${1+"$*"}
save_ifs="$IFS"
IFS=":"
set - $PATH
IFS="$save_ifs"

autoconf=autoconf
autoconf_vers=0
automake=automake
automake_vers=0
aclocal=aclocal
aclocal_vers=0
autoheader=autoheader
autoheader_vers=0
libtoolize=libtoolize
libtoolize_vers=0

here=`pwd`
while [ $# -ne 0 ] ; do
  dir=$1
  shift
  if [ ! -d $dir ]; then
    continue
  fi
  cd $dir

  # This should really be a shell function called 3 times
  progs=`ls autoconf* 2>/dev/null`
  if [ "X$progs" != "X" ]; then
    for prog in $progs; do
      vers=`$prog --version | sed -ne '1s/^.* //p'`
      if [ "X$vers" = "X" ]; then
        continue
      fi
      if expr $vers '>' $autoconf_vers >/dev/null; then
        autoconf=$prog
        autoconf_vers=$vers
      fi
    done
  fi
  progs=`ls automake* 2>/dev/null`
  if [ "X$progs" != "X" ]; then
    for prog in $progs; do
      vers=`$prog --version | sed -ne '1s/^.* //p'`
      if [ "X$vers" = "X" ]; then
        continue
      fi
      if expr $vers '>' $automake_vers >/dev/null; then
        automake=$prog
        automake_vers=$vers
      fi
    done
  fi
  progs=`ls aclocal* 2>/dev/null`
  if [ "X$progs" != "X" ]; then
    for prog in $progs; do
      vers=`$prog --version | sed -ne '1s/^.* //p'`
      if [ "X$vers" = "X" ]; then
        continue
      fi
      if expr $vers '>' $aclocal_vers >/dev/null; then
        aclocal=$prog
        aclocal_vers=$vers
      fi
    done
  fi
  progs=`ls libtoolize* 2>/dev/null`
  if [ "X$progs" != "X" ]; then
    for prog in $progs; do
      vers=`$prog --version | sed -ne '1s/^.* //p'`
      if [ "X$vers" = "X" ]; then
        continue
      fi
      if expr $vers '>' $libtoolize_vers >/dev/null; then
        libtoolize=$prog
        libtoolize_vers=$vers
      fi
    done
  fi
  progs=`ls autoheader* 2>/dev/null`
  if [ "X$progs" != "X" ]; then
    for prog in $progs; do
      vers=`$prog --version | sed -ne '1s/^.* //p'`
      if [ "X$vers" = "X" ]; then
        continue
      fi
      if expr $vers '>' $autoheader_vers >/dev/null; then
        autoheader=$prog
        autoheader_vers=$vers
      fi
    done
  fi
done
cd $here

set - $save_args
# END Find autoconf and automake


echo "$program: Found autoconf $autoconf version $autoconf_vers" 1>&2
echo "$program: Found automake $automake version $automake_vers" 1>&2
echo "$program: Found aclocal $aclocal version $aclocal_vers" 1>&2
echo "$program: Found autoheader $autoheader version $autoheader_vers" 1>&2
echo "$program: Found libtoolize $libtoolize version $libtoolize_vers" 1>&2


if ($autoconf --version) < /dev/null > /dev/null 2>&1 ; then
    if ($autoconf --version | awk 'NR==1 { if( $3 >= '$autoconf_min_vers') \
			       exit 1; exit 0; }');
    then
       echo "$program: ERROR: \`$autoconf' is too old."
       echo "           (version $autoconf_min_vers or newer is required)"
       DIE="yes"
    fi
else
    echo
    echo "$program: ERROR: You must have \`$autoconf' installed to compile $PACKAGE."
    echo "           (version $autoconf_min_vers or newer is required)"
    DIE="yes"
fi

if ($automake --version) < /dev/null > /dev/null 2>&1 ; then
  if ($automake --version | awk 'NR==1 { if( $4 >= '$automake_min_vers') \
			     exit 1; exit 0; }');
     then
     echo "$program: ERROR: \`$automake' is too old."
     echo "           (version $automake_min_vers or newer is required)"
     DIE="yes"
  fi
  if ($aclocal --version) < /dev/null > /dev/null 2>&1; then
    if ($aclocal --version | awk 'NR==1 { if( $4 >= '$aclocal_min_vers' ) \
						exit 1; exit 0; }' );
    then
      echo "$program: ERROR: \`$aclocal' is too old."
      echo "           (version $aclocal_min_vers or newer is required)"
      DIE="yes"
    fi
  else
    echo
    echo "$program: ERROR: Missing \`$aclocal'"
    echo "           The version of $automake installed doesn't appear recent enough."
    DIE="yes"
  fi
else
    echo
    echo "$program: ERROR: You must have \`$automake' installed to compile $PACKAGE."
    echo "           (version $automake_min_vers or newer is required)"
    DIE="yes"
fi

if ($libtoolize --version) < /dev/null > /dev/null 2>&1 ; then
    if ($libtoolize --version | awk 'NR==1 { if( $4 >= '$libtoolize_min_vers') \
			       exit 1; exit 0; }');
    then
       echo "$program: ERROR: \`$libtoolize' is too old."
       echo "           (version $libtoolize_min_vers or newer is required)"
       DIE="yes"
    fi
else
    echo
    echo "$program: ERROR: You must have \`libtoolize' installed to compile $PACKAGE."
    echo "           (version $libtoolize_min_vers or newer is required)"
    DIE="yes"
fi


if test "X$DIE" != X; then
  exit 1
fi

if test -z "$*"; then
  echo "$program: WARNING: Running \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo "\`$0' command line."
fi

am_opt=

config_dir=
if test -d $CONFIG_DIR; then
  config_dir=`cd $CONFIG_DIR; pwd`
fi

for coin in `find $SRCDIR -name configure.ac -print`
do 
  dir=`dirname $coin`
  if test -f $dir/NO-AUTO-GEN; then
    echo $program: Skipping $dir -- flagged as no auto-gen
  else
    echo $program: Processing directory $dir
    ( cd $dir

      # Ensure that these are created by the versions on this system
      # (indirectly via automake)
      rm -f ltconfig ltmain.sh libtool stamp-h*
      # Made by automake
      rm -f missing depcomp
      # automake junk
      rm -rf autom4te*.cache

      if test "X$config_dir" != X; then
        echo "$program: Updating config.guess and config.sub"
	for file in config.guess config.sub; do
	  cfile=$config_dir/$file
	  if test -f $cfile; then
	    $DRYRUN rm -f $file
	    $DRYRUN cp -p $cfile $file
	  fi
	done
      fi

      echo "$program: Running $libtoolize --copy --automake"
      $DRYRUN rm -f ltmain.sh libtool
      $DRYRUN $libtoolize --copy --automake

      echo "$program: Running $aclocal $aclocal_args"
      $DRYRUN $aclocal $aclocal_args
      if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
	echo "$program: Running $autoheader"
	$DRYRUN $autoheader
      fi
      echo "$program: Running $automake $am_opt"
      $DRYRUN $automake $automake_args $am_opt
      echo "$program: Running $autoconf"
      $DRYRUN $autoconf $autoconf_args
    )
  fi
done

rm -f config.cache

conf_flags="--enable-maintainer-mode"

AUTOMAKE=$automake
AUTOCONF=$autoconf
ACLOCAL=$aclocal
export AUTOMAKE AUTOCONF ACLOCAL

echo "$program: Running ./configure $conf_flags $@"
if test "X$DRYRUN" = X; then
  $DRYRUN ./configure $conf_flags "$@" \
  && echo "$program: Now type \`make' to compile $PACKAGE" || exit 1
else
  $DRYRUN ./configure $conf_flags "$@"
fi
