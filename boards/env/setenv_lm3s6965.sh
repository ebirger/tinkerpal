#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_LM3S6965 != "y" ]] ; then

    echo "Target configuration is not LM3S6965";
    echo "Please run 'make lm3s6965_gcc_defconfig'";

elif [[ $CONFIG_GCC == "y" ]] ; then

    TOOLCHAIN_DIR=/usr/local/tinkerpal/sat

    export BSPS_DIR=/usr/local/tinkerpal
    export LIBC_DIR=$TOOLCHAIN_DIR/arm-none-eabi/lib/thumb/cortex-m3
    export CROSS_COMPILE=$TOOLCHAIN_DIR/bin/arm-none-eabi-

elif [[ $CONFIG_TI_CCS5 == "y" ]] ; then

    # Change this to the approproate toolchain location
    export TI_CCS5=c:/TI/ccsv5
    export STELLARISWARE_DIR=e:/Programs/StellarisWare

fi
