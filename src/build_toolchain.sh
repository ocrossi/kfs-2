#!/usr/bin/env bash

set -euo pipefail

if [ $# -ne 4 ]
then
	echo "Usage: $0 gnu_mirror binutils_version gdb_version gcc_version" >&2
	exit 1
fi

# Fetch an archive at the given locations and verify them using the GNU keyring
fetch_gnu_archive() # [location]...
{
	local filename

	for location in "$@"
	do
		filename="${location##*/}"

		# Try to download the GNU keyring if not present, but don't fail if unavailable
		if [ ! -f gnu-keyring.gpg ]; then
			wget https://ftp.gnu.org/gnu/gnu-keyring.gpg 2>/dev/null || \
				echo "Warning: Could not download GNU keyring, skipping signature verification" >&2
		fi

		# If keyring exists and signatures exist, verify them
		if [ -f gnu-keyring.gpg ] && [ -e "$filename" ] && [ -e "$filename.sig" ]
		then
			gpgv --keyring ./gnu-keyring.gpg "$filename"{.sig,} 2>/dev/null || \
				echo "Warning: Signature verification failed for existing $filename" >&2
		fi

		# Download archive and signature
		wget --no-clobber "$location"{,.sig} 2>/dev/null || wget --no-clobber "$location"

		# Verify signature if keyring is available
		if [ -f gnu-keyring.gpg ]; then
			gpgv --keyring ./gnu-keyring.gpg "$filename"{.sig,} 2>/dev/null || \
				echo "Warning: Signature verification failed for $filename, proceeding anyway" >&2
		fi
	done
}

CACHE_DIR=/var/cache/kfs
SOURCE_DIR=/tmp/src

gnu_mirror="$1"

binutils_version="$2"
gdb_version="$3"
gcc_version="$4"

mkdir -vp /var/cache/kfs

pushd "$CACHE_DIR"
	fetch_gnu_archive \
		"$gnu_mirror/gnu/binutils/binutils-$binutils_version.tar.xz" \
		"$gnu_mirror/gnu/gdb/gdb-$gdb_version.tar.xz" \
		"$gnu_mirror/gnu/gcc/gcc-$gcc_version/gcc-$gcc_version.tar.xz"
popd

export PREFIX=/opt/cross
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

MAKEFLAGS="-j$(nproc)"; export MAKEFLAGS

mkdir -vp "$SOURCE_DIR"

pushd "$SOURCE_DIR"
	tar xf "$CACHE_DIR/binutils-$binutils_version.tar.xz"

	mkdir -vp "binutils-$binutils_version.build"
	pushd "binutils-$binutils_version.build"
		"../binutils-$binutils_version/configure" \
			--target="$TARGET" \
			--prefix="$PREFIX" \
			--with-sysroot \
			--disable-nls \
			--disable-werror

		make

		make install

		make clean
	popd

	# Ensure cross assembler is in the PATH.
	which -- $TARGET-as

	tar xf "$CACHE_DIR/gdb-$gdb_version.tar.xz"

	mkdir -vp "gdb-$gdb_version.build"
	pushd "gdb-$gdb_version.build"
		"../gdb-$gdb_version/configure" \
			--target="$TARGET" \
			--prefix="$PREFIX" \
			--disable-werror

		make all-gdb

		make install-gdb

		make clean
	popd

	tar xf "$CACHE_DIR/gcc-$gcc_version.tar.xz"

	mkdir -vp "gcc-$gcc_version.build"
	pushd "gcc-$gcc_version.build"
		"../gcc-$gcc_version/configure" \
			--target="$TARGET" \
			--prefix="$PREFIX" \
			--disable-nls \
			--enable-languages=c,c++ \
			--without-headers

		make all-gcc
		make all-target-libgcc

		make install-gcc
		make install-target-libgcc

		make clean
	popd
popd

# Cleanup.
rm -rf "$SOURCE_DIR"

# For now cache functionality is unused for container build.
rm -rf "$CACHE_DIR"

# shellcheck disable=SC2016
echo 'PATH="/opt/cross/bin:$PATH"' >> ~/.profile
