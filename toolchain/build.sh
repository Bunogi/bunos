#!/usr/bin/env bash
set -euo pipefail

typeset -r BINUTILS_VERSION=2.35
typeset -r BINUTILS_DOWNLOAD_URL=https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
typeset -r GCC_VERSION=10.2.0
typeset -r GCC_DOWNLOAD_URL=https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz

BINUTILS_TAR_FILE=$(basename $BINUTILS_DOWNLOAD_URL)
GCC_TAR_FILE=$(basename $GCC_DOWNLOAD_URL)

export TARGET=i686-elf
export PREFIX="$PWD/prefix"
export PATH="$PREFIX/bin:$PATH"
#export SYSROOT="$PWD/../sysroot"

# GCC is stinky and expects this to exists
mkdir -p $PREFIX/$TARGET/sys-root/usr/include
mkdir -p work

echo $PREFIX
echo $TARGET

#mkdir -p $PREFIX/include
#mkdir -p $PREFIX/lib

function build_binutils() {
    local BINUTILS_BUILD_DIR=$PWD/work/build-binutils
    rm -rf work/*binutils*
    if [ ! -f $BINUTILS_TAR_FILE ]; then
        curl $BINUTILS_DOWNLOAD_URL > $BINUTILS_TAR_FILE
    fi

    (cd work && tar xvf ../$BINUTILS_TAR_FILE)

    mkdir ${BINUTILS_BUILD_DIR}
    pushd .
    cd ${BINUTILS_BUILD_DIR}
    ../binutils-${BINUTILS_VERSION}/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
    make -j17
    make install
    popd
}

function build_gcc() {
    local GCC_BUILD_DIR=$PWD/work/build-gcc
    rm -rf work/*gcc*
    if [ ! -f $GCC_TAR_FILE ]; then
        curl $GCC_DOWNLOAD_URL > $GCC_TAR_FILE
    fi

    (cd work && tar xvf ../$GCC_TAR_FILE)
    
    mkdir $GCC_BUILD_DIR
    pushd .
    cd $GCC_BUILD_DIR
    ../gcc-${GCC_VERSION}/configure --target=$TARGET --prefix="$PREFIX" \
        --disable-nls \
	--with-gnu-as \
	--with-as="$PREFIX/bin/i686-elf-as" \
	--with-gnu-ld \
	--with-ld="$PREFIX/bin/i686-elf-ld" \
        --enable-languages=c,c++ \
        --without-headers
    make all-gcc -j17
    make all-target-libgcc -j17
    make install-gcc
    make install-target-libgcc -j17
    popd
}

build_binutils
build_gcc
