#!/usr/bin/env bash
set -euo pipefail

typeset -r this_dir=$(dirname $(realpath $0))
cd $this_dir

if [ -f build_host ]; then
    mkdir build_host
fi

cd build_host
cmake .. -DBUNOS_BUILD_HOST=TRUE
cd ..
if [ -f compile_commands.json ]; then
    rm compile_commands.json
fi
ln -s build_host/compile_commands.json .
