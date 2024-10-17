#!/usr/bin/env sh

./compile_commands_target.sh
clang-tidy $(git ls-files '*.cpp' '*.hpp' '*.h') $@
