#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_FRDM_KL25Z != "y" ]] ; then

    echo "Target configuration is not FRDM-KL25Z";
    echo "Please run 'make kl25z_gcc_defconfig'";

elif [[ $CONFIG_GCC == "y" ]] ; then

    TOOLCHAIN_DIR=/usr/local/tinkerpal/sat

    export LIBC_DIR=$TOOLCHAIN_DIR/arm-none-eabi/lib/thumb/cortex-m0
    export LIBGCC_DIR=$TOOLCHAIN_DIR/lib/gcc/arm-none-eabi/4.7.3/thumb/cortex-m0
    export CROSS_COMPILE=$TOOLCHAIN_DIR/bin/arm-none-eabi-

fi
