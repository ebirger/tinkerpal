STM32_DIR?=$(BSPS_DIR)
MK_SUBDIRS+=$(if $(CONFIG_STM32F1XX),stm32f1xx)
MK_SUBDIRS+=$(if $(CONFIG_STM32F3XX),stm32f3xx)
MK_SUBDIRS+=$(if $(CONFIG_STM32F4XX),stm32f4xx)

MK_OBJS+=stm32.o stm32_usart.o
MK_OBJS+=$(if $(CONFIG_GPIO),stm32_gpio.o)
MK_OBJS+=$(if $(CONFIG_SPI),stm32_spi.o)
MK_OBJS+=$(if $(CONFIG_STM32_FSMC_LCD),stm32_fsmc_lcd.o)

ifneq ($(CONFIG_GCC),)
  ifeq ($(CROSS_COMPILE),)
    $(error Please set CROSS_COMPILE environment variable)
  endif
  MK_OBJS+=stm32_entry_gcc.o
  INC:=-include $(d)/stm32_conf.h
  CFLAGS+=$(INC)
endif
