BSP_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32F4xx_StdPeriph_Driver

INCS:=-I$(BSP_DRIVERS_DIR)/inc \
   -I$(BSP_DIR)/Libraries/CMSIS/Device/ST/STM32F4xx/Include \
   -I$(BSP_DIR)/Libraries/CMSIS/Include

BSP_SRCS:=stm32f4xx_usart.c stm32f4xx_rcc.c stm32f4xx_gpio.c misc.c

MK_LINKS:=$(addprefix $(BSP_DRIVERS_DIR)/src/,$(BSP_SRCS))

MK_OBJS=stm32f4discovery.o system_stm32f4xx.o interrupts.o $(BSP_SRCS:%.c=%.o)

ifneq ($(CONFIG_GCC),)
  LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f4discovery.ld

  CFLAGS+=-mthumb -march=armv7e-m -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -O3 \
    -ffunction-sections -fdata-sections -fsingle-precision-constant \
    -DSTM32F4XX -DSTM32F40_41xxx $(INCS) -mlittle-endian

  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(LIBC_DIR) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
