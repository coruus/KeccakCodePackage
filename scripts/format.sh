#!/usr/bin/env sh
parallel clang-format-3.5 -style=file -i {} ::: $(gfind . -iname "*.c" -or -iname "*.h" -or -iname "*.macros")
