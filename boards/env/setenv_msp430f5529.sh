#!/bin/bash

host_os=`uname`;

source build.$host_os/.config

if [[ $CONFIG_MSP430F5529 != "y" ]] ; then

    echo "Target configuration is not MSP430F5529";
    echo "Please run 'make msp430f5529_defconfig'";

elif [[ $CONFIG_TI_CCS5 == "y" ]] ; then

    # Change this to the approproate toolchain location
    export TI_CCS5=C:/TI/ccsv5

fi
