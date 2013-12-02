MK_SUBDIRS+=$(if $(CONFIG_STM32F3DISCOVERY),stm32f3discovery)
MK_SUBDIRS+=$(if $(CONFIG_STM32F4DISCOVERY),stm32f4discovery)

MK_OBJS+=stm32.o $(if $(CONFIG_GPIO),stm32_gpio.o)

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
  MK_OBJS+=stm32_entry_gcc.o
  INC:=-include $(d)/stm32_conf.h
  CFLAGS+=$(INC)
endif
