#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_STM32_ARMJISHU_28 != "y" ]] ; then

    echo "Target configuration is not for the CONFIG_STM32_ARMJISHU_28 board";
    echo "Please run 'make stm32_armjishu_28_gcc_defconfig'";

elif [[ $CONFIG_GCC == "y" ]] ; then

    TOOLCHAIN_DIR=/usr/local/tinkerpal/sat
    
    export BSPS_DIR=/usr/local/tinkerpal
    export LIBC_DIR=$TOOLCHAIN_DIR/arm-none-eabi/lib/thumb/cortex-m3
    export CROSS_COMPILE=$TOOLCHAIN_DIR/bin/arm-none-eabi-

else

    echo "No support for non GCC builds on this platform yet";

fi
