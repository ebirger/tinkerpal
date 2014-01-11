#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_STM32F4DISCOVERY != "y" ]] ; then

    echo "Target configuration is not STM32F4DISCOVERY";
    echo "Please run 'make stm32f4discovery_gcc_defconfig'";

elif [[ $CONFIG_GCC == "y" ]] ; then

    TOOLCHAIN_DIR=/usr/local/tinkerpal/sat
    
    export BSP_DIR=/usr/local/tinkerpal/stm32_f4
    export LIBC_DIR=$TOOLCHAIN_DIR/arm-none-eabi/lib/thumb/cortex-m4
    export LIBGCC_DIR=$TOOLCHAIN_DIR/lib/gcc/arm-none-eabi/4.7.3/thumb/cortex-m4
    export CROSS_COMPILE=$TOOLCHAIN_DIR/bin/arm-none-eabi-

else

    echo "No support for non GCC builds on this platform yet";

fi
