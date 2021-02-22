#!/usr/bin/env bash
set -euo pipefail

typeset -r this_dir=$(dirname $(realpath $0))
cd $this_dir

if [ -f build ]; then
    mkdir build
fi

cd build
cmake .. 
cd ..
if [ -f compile_commands.json ]; then
    rm compile_commands.json
fi
ln -s build/compile_commands.json .
