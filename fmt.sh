#!/bin/bash
#
# Runs clang format over the whole project tree, excluding
# the 'externals/' and 'build/' directories.
#

find . -type d \( -path './externals' -o -path './build' \) -prune -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i

