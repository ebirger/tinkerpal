TinkerPal
=========
Lightweight JavaScript engine and ecosystem for embedded platforms

Platforms
---------
- TI Stellaris Launchpad (LM4F120XL)
- STM32F3Discovery
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
- Proper Vagrant environment is in the making...

Building
--------
```
> make menuconfig
> make
```

### Environment Variables
- CROSS_COMPILE - compiler prefix
- Other platform specific variables are required. See targets/

Building for a specific target
------------------------------
```
> make <defconfig file>
Where defconfig files can be found in targets/
> . ./targets/setenv_<target>.sh
> make
```

### Examples

Building as a Linux executable
```
> make unix_sim_gcc_defconfig
> . ./targets/setenv_unix_sim.sh
> make
```
The output executable can be found at ./build.Linux/tp

Building for the Stellaris Launchpad on Linux
```
> make lm4f120xl_gcc_defconfig
> . ./targets/setenv_lm4f120xl.sh
> make
```

The output file can be found at ./build.Linux/tp.bin
