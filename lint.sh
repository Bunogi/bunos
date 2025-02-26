#!/usr/bin/env bash

set -x
files=""

typeset -r USAGE="Usage: $0 [host|target] [file...] -- [clang-tidy options]"

if [ "$#" -lt 1 ]; then
    echo $USAGE
    exit 1
fi

case "$1" in
    "host"|"target")
        typeset -r target="$1"
        shift
        ;;
    *)
        echo $USAGE
        exit 1
        ;;
esac


while [ "$#" -ne 0 ]; do
    if [ "$1" == '--' ]; then
        shift;
        break
    fi
    files="$files $1"
    shift;
done


if [ "x$files" = "x" ]; then
    if [ "$target" = "host" ]; then
        files=$(git ls-files '*.cpp' '*.hpp' '*.h' | grep -v '^kernel')
    else
        files=$(git ls-files '*.cpp' '*.hpp' '*.h' | grep -v '^kernel')
    fi
fi


#./compile_commands_target.sh
#clang-tidy $files $@

#./compile_commands_host.sh

clang-format -i $files
exec clang-tidy --quiet $files $@
