BSP_DIR=$(STM32_DIR)/stm32_f1
BSP_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32F10x_StdPeriph_Driver

INCS:=-I$(BSP_DRIVERS_DIR)/inc \
   -I$(BSP_DIR)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/ \
   -I$(BSP_DIR)/Libraries/CMSIS/CM3/CoreSupport/

BSP_SRCS:=stm32f10x_usart.c stm32f10x_rcc.c misc.c
BSP_SRCS+=$(if $(CONFIG_GPIO),stm32f10x_gpio.c)
BSP_SRCS+=$(if $(CONFIG_SPI),stm32f10x_spi.c)

MK_LINKS:=$(addprefix $(BSP_DRIVERS_DIR)/src/,$(BSP_SRCS))

MK_OBJS=stm32f103xx.o system_stm32f10x.o interrupts.o $(BSP_SRCS:%.c=%.o)

ifneq ($(CONFIG_GCC),)
  ifneq ($(CONFIG_STM32F103RBT),)
    LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f103rbt.ld
  endif

  CFLAGS+=-mthumb -mcpu=cortex-m3 -O3 -mfix-cortex-m3-ldrd  -mthumb-interwork \
    -mfloat-abi=soft -ffunction-sections -fdata-sections \
    -fsingle-precision-constant -DSTM32F1 -DSTM32F10X_MD $(INCS) -mlittle-endian
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(call get_libc_dir) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
