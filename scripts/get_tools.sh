#!/bin/bash -e

basedir=$1

if [ -z "$basedir" ] ; then
    echo "Usage $0 <basedir>";
    exit 1;
fi

function install_lm4tools()
{
    if [ -d lm4tools ] ; then
	echo "lm4tools found at $basedir/lm4tools";
    	return;
    fi

    read -p "Fetch and build lm4tools? (y/n): " get_lm4tools;
    if [[ $get_lm4tools != "y" ]] ; then
	return;
    fi
    echo "Fetching lm4tools";
    git clone https://github.com/utzig/lm4tools.git;
    pushd lm4tools/lm4flash;
    echo "Building lm4tools";
    make;
    popd;
}

function install_arm_toolchain() 
{
    if [ -d sat/bin ] ; then
	echo "ARM toolchain found at $basedir/sat/bin";
    	return;
    fi

    echo "Fetching ARM toolchain";
    wget -c http://www.tinkerpal.org/sat.tgz -O - | tar xz;
}

function install_stellarisware()
{
    if [ -d stellarisware ] ; then
	echo "Stellarisware dependencies found at $basedir/stellarisware";
    	return;
    fi

    echo "Fetching Stellarisware dependencies";
    wget -c http://www.tinkerpal.org/stellarisware.tgz -O - | tar xz;
}

mkdir -p $basedir

pushd $basedir;

function atexit_func 
{
    # get back to where we were
    popd
}
trap "atexit_func" EXIT

install_lm4tools;
install_arm_toolchain;
install_stellarisware;
