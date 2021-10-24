#!/usr/bin/env bash
set -euo pipefail

typeset -r BINUTILS_VERSION=2.35
typeset -r BINUTILS_DOWNLOAD_URL=https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
typeset -r GCC_VERSION=11.2.0
typeset -r GCC_DOWNLOAD_URL=https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz

typeset -r BINUTILS_SUCCESSFUL_FILE="$PWD/binutils.success"

BINUTILS_TAR_FILE=$(basename $BINUTILS_DOWNLOAD_URL)
GCC_TAR_FILE=$(basename $GCC_DOWNLOAD_URL)

export TARGET=i686-bunos
export PREFIX="$PWD/prefix"
export SYSROOT="$PWD/../fsroot"

typeset -r BUNOS_SOURCE_DIR="$PWD/.."

mkdir -p work

echo $PREFIX
echo $TARGET

typeset -i makejobs=$(nproc)
makejobs=${makejobs}+1
echo $makejobs

typeset -r binutils_patch=$PWD/binutils.patch
typeset -r gcc_patch=$PWD/gcc.patch

#mkdir -p $PREFIX/include
#mkdir -p $PREFIX/lib

function build_binutils() {
    local -r BINUTILS_BUILD_DIR=$PWD/work/build-binutils
    # Would be nice to not build binutils twice
    if [ -e $BINUTILS_BUILD_DIR ] && [-e $BINUTILS_SUCCESSFUL_FILE ]; then
        return
    fi
    rm -rf work/*binutils*
    rm -rf $BINUTILS_SUCCESSFUL_FILE
    if [ ! -f $BINUTILS_TAR_FILE ]; then
        curl $BINUTILS_DOWNLOAD_URL > $BINUTILS_TAR_FILE
    fi

    (cd work && tar xvf ../$BINUTILS_TAR_FILE && patch -p1 -ruN -d binutils-${BINUTILS_VERSION} < ${binutils_patch})
    #patch -ruN -d $PWD/work/binutils-${BINUTILS_VERSION} < ${binutils_patch}

    mkdir ${BINUTILS_BUILD_DIR}
    pushd .

    # The patch messes with ld, so we have to re-run automake
    pushd .
    cd work/binutils-${BINUTILS_VERSION}/ld
    aclocal
    automake
    popd

    cd ${BINUTILS_BUILD_DIR}
    ../binutils-${BINUTILS_VERSION}/configure \
        --target=$TARGET \
        --prefix=$PREFIX \
        --with-sysroot=$SYSROOT \
        --disable-nls \
        --disable-werror
    make -j ${makejobs}
    make install
    popd

    touch $BINUTILS_SUCCESSFUL_FILE
}

function build_gcc() {
    local GCC_BUILD_DIR=$PWD/work/build-gcc
    rm -rf work/*gcc*
    if [ ! -f $GCC_TAR_FILE ]; then
        curl $GCC_DOWNLOAD_URL > $GCC_TAR_FILE
    fi

    (cd work && tar xvf ../$GCC_TAR_FILE && patch -p1 -ruN -d gcc-${GCC_VERSION} < ${gcc_patch})

    pushd .
    cd work/gcc-${GCC_VERSION}/libstdc++-v3
    #autoconf # Unsure if needed, seems to compile fine without this
    popd
    
    mkdir $GCC_BUILD_DIR
    pushd .
    cd $GCC_BUILD_DIR
    ../gcc-${GCC_VERSION}/configure --target=$TARGET --prefix="$PREFIX" \
        --disable-nls \
        --with-gnu-as \
        --with-as="$PREFIX/bin/$TARGET-as" \
        --with-gnu-ld \
        --with-ld="$PREFIX/bin/$TARGET-ld" \
        --with-sysroot=$SYSROOT \
        --enable-languages=c,c++

    $BUNOS_SOURCE_DIR/copy_libc_headers.sh

    make all-gcc -j ${makejobs}
    make all-target-libgcc -j ${makejobs}

    make install-gcc -j ${makejobs} 
    make install-target-libgcc -j ${makejobs}
    popd
}

build_binutils
build_gcc
