#!/bin/bash
host_os=`uname`;
target=$1;
binpath=$2;
version=$3;
binfile_suffix=$4;

echo "Building $target $version";

make ${target}_gcc_defconfig;
source build.$host_os/.config;
source ./targets/setenv_${target}.sh;
make;
cp build.$host_os/tp${binfile_suffix} $binpath/tinkerpal-v$version-${target}${binfile_suffix};
make distclean;
