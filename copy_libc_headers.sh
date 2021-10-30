#!/usr/bin/env bash
set -euo pipefail

typeset -r this_dir=$(dirname $(realpath $0))
cd $this_dir

dirs=$(cd libc && find . -type d)
for i in $dirs; do
    dir=fsroot/usr/include/$i
    if [[ -f "$dir" ]]; then
        echo "Not making $dir: already exists" || true
        continue;
    fi
    echo "Making $dir" || true
    mkdir -p $dir
done

headers=$(cd libc && find . -name '*.h')
for i in $headers; do
    source_file=libc/$i
    dest_file=fsroot/usr/include/$i
    if [ "$source_file" -ot "$dest_file" ]; then
        echo "skipping $dest_file: not older than $source_file"
        continue;
    fi
    echo "libc/$i -> fsroot/usr/include/$i"
    cp libc/$i fsroot/usr/include/$i
done
