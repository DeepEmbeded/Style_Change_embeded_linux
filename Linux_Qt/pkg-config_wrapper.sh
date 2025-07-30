#!/bin/sh
PKG_CONFIG_SYSROOT_DIR=/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/aarch64-buildroot-linux-gnu/sysroot
export PKG_CONFIG_SYSROOT_DIR
PKG_CONFIG_LIBDIR=/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/aarch64-buildroot-linux-gnu/sysroot/usr/lib/pkgconfig:/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/aarch64-buildroot-linux-gnu/sysroot/usr/share/pkgconfig:/home/elf/aarch64-buildroot-linux-gnu_sdk-buildroot/aarch64-buildroot-linux-gnu/sysroot/usr/lib/aarch64-buildroot-linux-gnu/pkgconfig
export PKG_CONFIG_LIBDIR
exec pkg-config "$@"
