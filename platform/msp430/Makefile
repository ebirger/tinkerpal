MK_OBJS=msp430f5529.o msp430f5529_usci.o
MK_OBJS+=$(if $(CONFIG_GPIO),msp430f5529_gpio.o)
MK_OBJS+=$(if $(CONFIG_SPI),msp430f5529_spi.o)

ifneq ($(CONFIG_DOGS102X6),)
  MK_OBJS+=dogs102x6.o $(if $(CONFIG_JS),js_dogs102x6.o)
  MK_DESCS=dogs102x6.desc
endif

TI_COMPILER:=$(TI_CCS5)/tools/compiler/msp430_4.1.1
CC=$(TI_COMPILER)/bin/cl430
LINKER_SCRIPT:=$(d)/lnk_msp430f5529.cmd 
CFLAGS+=$(if $(POWER_DBG),--advice:power=all)
CFLAGS+=-vmspx --abi=eabi --define=__MSP430F5529__ \
  --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 \
  --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal \
  --include_path="$(TI_CCS5)/ccs_base/msp430/include"
LDFLAGS+=-vmspx --abi=eabi -g -i"$(TI_CCS5)/ccs_base/msp430/include"
LIBS+=-l"libc.a"
