BSP_DRIVERS_DIR:=$(BSP_DIR)/Libraries/STM32F30x_StdPeriph_Driver

INCS:=-I$(BSP_DRIVERS_DIR)/inc \
   -I$(BSP_DIR)/Libraries/CMSIS/Device/ST/STM32F30x/Include \
   -I$(BSP_DIR)/Libraries/CMSIS/Include

BSP_SRCS:=stm32f30x_usart.c stm32f30x_rcc.c stm32f30x_gpio.c stm32f30x_misc.c

MK_LINKS:=$(addprefix $(BSP_DRIVERS_DIR)/src/,$(BSP_SRCS))

MK_OBJS=stm32f3discovery.o system_stm32f30x.o interrupts.o $(BSP_SRCS:%.c=%.o)

ifneq ($(CONFIG_GCC),)
  LINKER_SCRIPT:=$(BUILD)/$(d)/stm32f3discovery.ld

  CFLAGS+=-mthumb -march=armv7e-m -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -O3 \
    -ffunction-sections -fdata-sections -fsingle-precision-constant \
    -DSTM32F3 $(INCS) -mlittle-endian
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(LIBC_DIR) -lm -lc -L$(LIBGCC_DIR) -lgcc
endif
