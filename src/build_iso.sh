#!/usr/bin/env bash

set -euo pipefail

iso_name=kfs-1.iso

iso_content_dir="sysroot"
iso_dist_dir="${DESTDIR:-/dist}"

MAKEFLAGS="-j$(nproc)"; export MAKEFLAGS

# shellcheck source=/dev/null
source "$HOME/.profile"

./build.sh

grub-mkrescue -v -o "$iso_dist_dir/$iso_name" "$iso_content_dir"
