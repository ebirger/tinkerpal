TinkerPal
=========
Lightweight JavaScript engine and ecosystem for embedded platforms

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
- Use ./scripts/get_tools.sh as root (recommended basedir /usr/local/tinkerpal,
  otherwise setenv_<target> scripts would need to be adjusted)
- You can also download a ready to use CD image file with all the required tools and pre-requisites from http://www.tinkerpal.org/tinkerpal.iso

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
