MK_OBJS=tm4c1294.o interrupts.o

ifneq ($(CONFIG_GCC),)
  BSP_SRCS:=systick.c sysctl.c gpio.c interrupt.c uart.c ssi.c cpu.c adc.c \
    pwm.c emac.c $(if $(CONFIG_I2C),i2c.c)
  
  MK_LINKS:=$(addprefix $(TI_ARM_MCU_BSP_DIR)/driverlib/,$(BSP_SRCS))
  MK_OBJS+=entry_gcc.o $(BSP_SRCS:%.c=%.o)

  LINKER_SCRIPT:=$(BUILD)/$(d)/tm4c1294.ld
  CFLAGS+=-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -O3 \
    -ffunction-sections -fdata-sections -I$(TI_ARM_MCU_BSP_DIR) \
    -DPART_TM4C1294NCPDT -Dgcc -fsingle-precision-constant
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(call get_libc_dir) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
