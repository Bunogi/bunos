#!/usr/bin/env bash
set -euo pipefail

typeset -r this_dir=$(dirname $(realpath $0))
cd $this_dir

dirs=$(cd libc && find . -type d)
for i in $dirs; do
    echo "Making fsroot/usr/include/$i" || true
    mkdir -p fsroot/usr/include/$i
done

headers=$(cd libc && find . -name '*.h')
for i in $headers; do
    echo "libc/$i -> fsroot/usr/include/$i"
    cp libc/$i fsroot/usr/include/$i
done
