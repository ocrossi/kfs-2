#!/usr/bin/env bash

set -euo pipefail

apt-get -y update

apt-get -y upgrade

apt-get -y install --no-install-recommends \
	build-essential bison flex libgmp3-dev libmpc-dev \
	libmpfr-dev texinfo libisl-dev \
	ca-certificates wget gpg xz-utils grub-pc xorriso

apt-get clean

rm -rf /var/lib/apt/lists/*
