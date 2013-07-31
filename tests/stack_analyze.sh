#!/bin/bash
valgrind --tool=massif --stacks=yes --massif-out-file=test.ms --time-unit=B ./js $1
ms_print test.ms
