# shellcheck shell=sh disable=SC2034

SYSTEM_HEADER_PROJECTS="libc kernel"
PROJECTS="libc kernel"

COMMONDIR="$(pwd)/common"; export COMMONDIR

export MAKE="${MAKE:-make}"
export HOST="${HOST:-$("$COMMONDIR"/default_host.sh)}"

export AR="${HOST}-ar"
export AS="${HOST}-as"
export CC="${HOST}-gcc"

export PREFIX=/usr
export EXEC_PREFIX="$PREFIX"

export BOOTDIR=/boot
export LIBDIR="$EXEC_PREFIX/lib"
export INCDIR="$PREFIX/include"

export CFLAGS='-O2 -g'
export CPPFLAGS=''

# Configure the cross-compiler to use the desired system root.
SYSROOT="$(pwd)/sysroot"; export SYSROOT
export CC="$CC --sysroot=$SYSROOT"

# Work around that the -elf gcc targets doesn't have a system include directory
# because it was configured with --without-headers rather than --with-sysroot.
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
  export CC="$CC -isystem=$INCDIR"
fi
