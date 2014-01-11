TinkerPal
=========
Lightweight JavaScript engine and ecosystem for embedded platforms

Platforms
---------
All supported platform provide a serial console, some provide GPIO and SPI support
- TI Stellaris LM4F120XL (“Stellaris Launchpad”)
- TI Stellaris LM3S6965
- TI Stellaris LM3S6918 (“RDK-IDM”)
- TI MSP-EXP430F5529 (“MSP430F5529 USB Experimenter’s Board”)
- TI MSP-EXP430F5529LP (“MSP430F5529 Launchpad”)
- ST STM32F3Discovery
- ST STM32F4Discovery
- Freescale FRDM-KL25Z
- Linux/Cygwin

Installation
------------
### Basic pre-requisites
- gcc
- make
- gperf
- bison
- libncurses5-dev

### lm4tools
- git-core
- libusb-1.0.0-dev
- pkg-config

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
- Other platform specific variables are required. See boards/env/

Building for a specific target
------------------------------
```
> make <defconfig file>
Where defconfig files can be found in boards/env/
> . ./boards/env/setenv_<target>.sh
> make
```

### Examples

Building as a Linux executable
```
> make unix_sim_gcc_defconfig
> . ./boards/env/setenv_unix_sim.sh
> make
```
The output executable can be found at ./build.Linux/tp

Building for the Stellaris Launchpad on Linux
```
> make lm4f120xl_gcc_defconfig
> . ./boards/env/setenv_lm4f120xl.sh
> make
```

The output file can be found at ./build.Linux/tp.bin
