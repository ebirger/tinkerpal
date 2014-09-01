#!/bin/bash

[ "$DEFCONFIG" == "unix_sim_tests_defconfig" ] && coveralls --exclude staging.Linux --exclude doc --exclude scripts --exclude tests --gcov-options '\-lp' -r . -b .
