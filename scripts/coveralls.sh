#!/bin/bash

[ "$DEFCONFIG" == "unix_sim_tests_defconfig" ] && {
    coveralls --exclude build.Linux/fs/fat/ff.c --exclude staging.Linux --exclude doc --exclude scripts --exclude tests --gcov-options '\-lp' -r . -b .
    bash <(curl -s https://codecov.io/bash)
}
