BSP_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32F4xx_StdPeriph_Driver

INCS:=-I$(BSP_DRIVERS_DIR)/inc \
   -I$(BSP_DIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include \
   -I$(BSP_DIR)/Libraries/CMSIS/Include

BSP_SRCS:=stm32f4xx_usart.c stm32f4xx_rcc.c misc.c
BSP_SRCS+=$(if $(CONFIG_GPIO),stm32f4xx_gpio.c)
BSP_SRCS+=$(if $(CONFIG_SPI),stm32f4xx_spi.c)

MK_LINKS:=$(addprefix $(BSP_DRIVERS_DIR)/src/,$(BSP_SRCS))

ifneq ($(CONFIG_STM32F407XX),)
MK_OBJS=stm32f407xx.o system_stm32f4xx.o interrupts.o
endif

ifneq ($(CONFIG_STM32F429XX),)
MK_OBJS=stm32f429xx.o system_stm32f429xx.o stm32f429xx_interrupts.o
endif

MK_OBJS+=$(BSP_SRCS:%.c=%.o)
ifneq ($(CONFIG_GCC),)
  ifneq ($(CONFIG_STM32F407XX),)
    LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f407xx.ld
  endif
  ifneq ($(CONFIG_STM32F429XX),)
    LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f429xx.ld
  endif

  CFLAGS+=-mthumb -march=armv7e-m -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -O3 \
    -ffunction-sections -fdata-sections -fsingle-precision-constant \
    -DSTM32F4XX -DSTM32F40_41xxx $(INCS) -mlittle-endian

  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBC_DIR:=$(LIBC_DIR)/float-abi-hard/fpuv4-sp-d16/
  LIBS+=-L$(LIBC_DIR) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
