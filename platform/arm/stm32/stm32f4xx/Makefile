BSP_DIR=$(STM32_DIR)/$(if $(CONFIG_STM32F429XX),stm32_f429,stm32_f4)
BSP_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32F4xx_StdPeriph_Driver
BSP_USBD_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32_USB_Device_Library/Core
BSP_USB_OTG_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32_USB_OTG_Driver

INCS:=-I$(BSP_DRIVERS_DIR)/inc \
   -I$(BSP_DIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include \
   -I$(BSP_DIR)/Libraries/CMSIS/Include

BSP_SRCS:=stm32f4xx_usart.c stm32f4xx_rcc.c misc.c
BSP_SRCS+=$(if $(CONFIG_GPIO),stm32f4xx_gpio.c)
BSP_SRCS+=$(if $(CONFIG_SPI),stm32f4xx_spi.c)
BSP_SRCS+=$(if $(CONFIG_I2C),stm32f4xx_i2c.c)
BSP_SRCS+=$(if $(CONFIG_STM32_FSMC_LCD),stm32f4xx_fsmc.c)

MK_LINKS:=$(addprefix $(BSP_DRIVERS_DIR)/src/,$(BSP_SRCS))
MK_OBJS+=$(BSP_SRCS:%.c=%.o)

ifneq ($(CONFIG_USB_DEVICE),)
  BSP_USB_SRCS:=usbd_ioreq.c
  BSP_USB_OTG_SRCS:=usb_dcd.c usb_core.c
  MK_CFLAGS_usb_core.o+=-fno-strict-aliasing
  INCS:=$(INCS) -I$(BSP_USBD_DRIVERS_DIR)/inc \
    -I$(BSP_USB_OTG_DRIVERS_DIR)/inc -I$(d)/usb_conf
  MK_LINKS:=$(MK_LINKS) \
    $(addprefix $(BSP_USBD_DRIVERS_DIR)/src/,$(BSP_USB_SRCS)) \
    $(addprefix $(BSP_USB_OTG_DRIVERS_DIR)/src/,$(BSP_USB_OTG_SRCS))
  MK_OBJS+=$(BSP_USB_SRCS:%.c=%.o) $(BSP_USB_OTG_SRCS:%.c=%.o)
endif

ifneq ($(CONFIG_STM32F407XX),)
  MK_OBJS+=stm32f407xx.o system_stm32f4xx.o interrupts.o
endif

ifneq ($(CONFIG_STM32F429XX),)
  MK_OBJS+=stm32f429xx.o system_stm32f429xx.o stm32f429xx_interrupts.o
endif

ifneq ($(CONFIG_GCC),)
  ifneq ($(CONFIG_STM32F407XX),)
    LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f407xx.ld
    CFLAGS:=$(CFLAGS) -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER -I$(d)
  endif
  ifneq ($(CONFIG_STM32F429XX),)
    LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f429xx.ld
    CFLAGS+=-DSTM32F429_439xx
  endif

  CFLAGS+=-mthumb -march=armv7e-m -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -O3 \
    -ffunction-sections -fdata-sections -fsingle-precision-constant \
    -DSTM32F4XX $(INCS) -mlittle-endian

  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(call get_libc_dir) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
