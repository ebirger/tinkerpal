MK_SUBDIRS+=$(if $(CONFIG_STM32F3XX),stm32f3xx)
MK_SUBDIRS+=$(if $(CONFIG_STM32F4XX),stm32f4xx)

MK_OBJS+=stm32.o
MK_OBJS+=$(if $(CONFIG_GPIO),stm32_gpio.o)
MK_OBJS+=$(if $(CONFIG_SPI),stm32_spi.o)

ifneq ($(CONFIG_GCC),)
  ifeq ($(CROSS_COMPILE),)
    $(error Please set CROSS_COMPILE environment variable)
  endif
  ifeq ($(LIBC_DIR),)
    $(error Please set LIBC environment variable)
  endif
  MK_OBJS+=stm32_entry_gcc.o
  INC:=-include $(d)/stm32_conf.h
  CFLAGS+=$(INC)
endif
