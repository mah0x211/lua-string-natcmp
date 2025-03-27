#!/usr/bin/env sh

set -ex

mkdir -p ./coverage
lcov -c -d ./src -o coverage/lcov.info.all
lcov --ignore-errors unused -r coverage/lcov.info.all '*/include/*' 'utf8clen.h' 'natcmp.h' -o coverage/lcov.info
genhtml -o coverage/html coverage/lcov.info
