#!/bin/bash

if [[ $# != 3 ]] ; then
    echo "Usage: $0 <where> <last> <new>";
    exit 1;
fi

where="$1"
last="$2"
new="$3"

host_os=`uname`;
binpath=$where/tinkerpal-v$new;

mkdir -p $binpath

# Source tar.gz
echo "git tag v$new"
echo "git archive --prefix=tinkerpal-$new/ v$new | gzip -9 > $where/tinkerpal-$new.tar.gz"
echo "git log --no-merges v$new ^v$last > $where/ChangeLog-$new"
echo "git shortlog --no-merges v$new ^v$last > $where/ShortLog"
echo "git diff --stat --summary -M v$last v$new > $where/diffstat-$new"

function build()
{
    target=$1;
    binfile_suffix=$2
    make ${target}_gcc_defconfig;
    source build.$host_os/.config;
    source ./targets/setenv_${target}.sh;
    make;
    cp build.$host_os/tp${binfile_suffix} $binpath/tinkerpal-v$new-${target}${binfile_suffix};
    make distclean;
}

build lm4f120xl .bin
build lm3s6965 .bin
build stm32f3discovery .bin
build unix_sim

zip $binpath.zip $binpath/*;
rm -rf $binpath;
