TinkerPal
=========
Lightweight JavaScript engine and ecosystem for embedded platforms

[Roadmap](https://trello.com/b/NReiA47h)

[![Build Status](https://travis-ci.org/ebirger/tinkerpal.png?branch=master)](https://travis-ci.org/ebirger/tinkerpal)
[![Coverage Status](https://coveralls.io/repos/ebirger/tinkerpal/badge.png?branch=master)](https://coveralls.io/r/ebirger/tinkerpal?branch=master)
[![codecov.io](https://codecov.io/github/ebirger/tinkerpal/coverage.svg?branch=master)](https://codecov.io/github/ebirger/tinkerpal?branch=master)
[![Coverity Scan Build](https://scan.coverity.com/projects/2946/badge.svg)](https://scan.coverity.com/projects/2946)
[![Documentation Status](https://readthedocs.org/projects/tinkerpal-docs/badge/?version=latest)](http://tinkerpal-docs.readthedocs.org/en/latest/?badge=latest)

Platforms
---------
All supported platform provide a serial console, some provide GPIO and SPI support
- TI Stellaris EK-LM4F120XL (“Stellaris Launchpad”)
- TI Stellaris EK-LM3S6965
- TI Stellaris LM3S6918 (“RDK-IDM”)
- TI Tiva C EK-TM4C123GXL (“Tiva C Launchpad”)
- TI Tiva C EK-TM4C1294XL ("Tiva C Connected Launchpad")
- TI MSP-EXP430F5529 (“MSP430F5529 USB Experimenter’s Board”)
- TI MSP-EXP430F5529LP (“MSP430F5529 Launchpad”)
- ST STM32F3Discovery
- ST STM32F4Discovery
- ST STM32F429IDiscovery
- HY 24" STM32F103 based board
- Freescale FRDM-KL25Z
- Linux/Cygwin

Installation
------------
### Basic pre-requisites
- gcc
- make
- gperf
- flex
- bison
- libncurses5-dev

### lm4tools
- git-core
- libusb-1.0.0-dev
- pkg-config

### mspdebug
- libreadline6

### ST Link
- autoconf

### STM32Loader
- python
- python-serial

### Runtime
screen / minicom / putty

Obtaining Toolchain / SDK
-------------------------
ARM cross compilation toolchain can be found at https://launchpad.net/gcc-arm-embedded
Alternatively, you can use an optimized toolchain from http://tinkerpal.org/sat.tgz

BSPs are downloaded automatically as part of the build process

Building
--------
```
> make menuconfig
> make
```

### Environment Variables
- CROSS_COMPILE - compiler prefix

Building for a specific target
------------------------------
```
> make <defconfig file>
Where defconfig files can be found in boards/configs/
> make
```

### Examples

Building as a Linux executable
```
> make unix_sim_gcc_defconfig
> make
```
The output executable can be found at ./build.Linux/tp

Building for the Stellaris Launchpad on Linux
```
> export CROSS_COMPILE=arm-none-eabi-
> make lm4f120xl_gcc_defconfig
> make
```

The output file can be found at ./build.Linux/tp.bin

Flashing TinkerPal
------------------
Most targets can be flashed by running 'make burn'. Note: this requires root
privileges
