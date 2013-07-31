#!/bin/bash

host_os=`uname`;
basedir=$1

if [ -z "$basedir" ] ; then
    echo "Usage $0 <basedir>";
    exit 1;
fi

sudo mkdir -p $basedir

function install_lm4tools()
{
    if [ -d $basedir/lm4tools ] ; then
	echo "lm4tools found at $basedir/lm4tools.";
    	return;
    fi
    read -p "Fetch and build lm4tools? (y/n): " get_lm4tools;
    if [[ $get_lm4tools != "y" ]] ; then
	return;
    fi
    pushd $basedir;
    echo "Fetching lm4tools";
    sudo git clone https://github.com/utzig/lm4tools.git;
    pushd lm4tools/lm4flash;
    echo "Building lm4tools";
    sudo make;
    popd;
    popd;
}

function install_arm_toolchain() 
{
    if [ -d $basedir/sat/bin ] ; then
	echo "ARM toolchain found at $toolchain_dir.";
    	return;
    fi

    echo "Fetching ARM toolchain";
    wget -c http://www.tinkerpal.org/sat.tgz;
    sudo mv sat.tgz $basedir;
    pushd $basedir;
    sudo tar -xzvf sat.tgz;
    sudo rm sat.tgz;
    popd;
}

function install_stellarisware()
{
    echo "Fetching Stellarisware dependencies";
    wget -c http://www.tinkerpal.org/stellarisware.tgz;
    sudo mv stellarisware.tgz $basedir;
    pushd $basedir;
    sudo tar -xzvf stellarisware.tgz;
    sudo rm stellarisware.tgz;
    popd;
}

install_lm4tools;
install_arm_toolchain;
install_stellarisware;
