#!/usr/bin/env bash

set -x
files=""

while [ "$#" -ne 0 ]; do
    if [ "$1" == '--' ]; then
        shift;
        break
    fi
    files="$files $1"
    shift;
done

if [ "x$files" = "x" ]; then
    files=$(git ls-files '*.cpp' '*.hpp' '*.h')
fi

./compile_commands_target.sh
clang-tidy $files $@
