#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_STM32F4DISCOVERY != "y" ]] ; then

    echo "Target configuration is not STM32F4DISCOVERY";
    echo "Please run 'make stm32f4discovery_gcc_defconfig'";

elif [[ $CONFIG_GCC == "y" ]] ; then

    TOOLCHAIN_DIR=/usr/local/tinkerpal/sat
    
    export BSPS_DIR=/usr/local/tinkerpal
    export CROSS_COMPILE=$TOOLCHAIN_DIR/bin/arm-none-eabi-

else

    echo "No support for non GCC builds on this platform yet";

fi
