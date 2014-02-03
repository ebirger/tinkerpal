#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_LM4F120XL != "y" ]] ; then

    echo "Target configuration is not LM4F120XL";
    echo "Please run 'make lm4f120xl_gcc_defconfig'";

elif [[ $CONFIG_GCC == "y" ]] ; then

    TOOLCHAIN_DIR=/usr/local/tinkerpal/sat

    export BSPS_DIR=/usr/local/tinkerpal
    export CROSS_COMPILE=$TOOLCHAIN_DIR/bin/arm-none-eabi-

elif [[ $CONFIG_TI_CCS5 == "y" ]] ; then

    # Change this to the approproate toolchain location
    export TI_CCS5=C:/TI/ccsv5
    export STELLARISWARE_DIR=E:/Programs/StellarisWare

fi
