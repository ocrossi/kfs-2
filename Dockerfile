FROM debian:bullseye-slim

# Prevent interaction (do not change)
ENV DEBIAN_FRONTEND=noninteractive

RUN mkdir /dist
VOLUME /dist

WORKDIR /root

COPY src/install_dependencies.sh /root
RUN ./install_dependencies.sh

ARG gnu_mirror=https://mirror.cyberbits.eu

ARG binutils_version=2.41
ARG gdb_version=14.1
ARG gcc_version=13.2.0

COPY src/build_toolchain.sh /root
RUN ./build_toolchain.sh \
    "$gnu_mirror" \
    "$binutils_version" "$gdb_version" "$gcc_version"

COPY src/. /root

CMD bash
