#!/bin/bash

if [[ $# != 3 ]] ; then
    echo "Usage: $0 <where> <last> <new>";
    exit 1;
fi

where="$1"
last="$2"
new="$3"

binpath=$where/tinkerpal-v$new

mkdir -p $binpath

git shortlog --no-merges v$last..v$new > $binpath/ShortLog
git diff --stat --summary -M v$last..v$new > $binpath/diffstat-v$new
git archive --prefix=tinkerpal-$new/ v$new | gzip -9 > $binpath/tinkerpal-v$new-src.tgz

function build()
{
    target=$1;
    binfile_suffix=$2
    ./scripts/build_dist.sh $target $binpath $new $binfile_suffix
}

build lm4f120xl .bin
build lm3s6965 .bin
build lm3s6918 .bin
build tm4c123g .bin
build tm4c1294 .bin
build stm32f3discovery .bin
build stm32f3discovery .bin
build stm32f429idiscovery .bin
build stm32f4discovery .bin
build frdm_kl25z .bin
build unix_sim

zip $binpath.zip $binpath/*;
rm -rf $binpath;
