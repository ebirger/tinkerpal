STELLARISWARE_DIR?=$(CONFIG_TI_STELLARISWARE_DIR:"%"=%)

MK_OBJS=stellaris.o 
MK_OBJS+=$(if $(CONFIG_TI_CCS5),stellaris_entry_ccs5.o)
MK_OBJS+=$(if $(CONFIG_GPIO),stellaris_gpio.o)
MK_OBJS+=$(if $(CONFIG_SPI),stellaris_spi.o)

MK_SUBDIRS+=$(if $(CONFIG_LM4F120XL),lm4f120xl)
MK_SUBDIRS+=$(if $(CONFIG_LM3S6965),lm3s6965)
MK_SUBDIRS+=$(if $(CONFIG_LM3S6918),lm3s6918)

ifneq ($(CONFIG_GCC),)
  ifeq ($(CROSS_COMPILE),)
    $(error Please set CROSS_COMPILE environment variable)
  endif
  ifeq ($(LIBC_DIR),)
    $(error Please set LIBC environment variable)
  endif
  ifeq ($(LIBGCC_DIR),)
    $(error Please set LIBGCC environment variable)
  endif
endif
