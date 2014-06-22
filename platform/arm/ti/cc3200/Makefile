MK_OBJS=cc3200.o interrupts.o

ifneq ($(CONFIG_GCC),)
  BSP_SRCS:=systick.c gpio.c interrupt.c uart.c pin.c cpu.c prcm.c
  
  MK_LINKS:=$(addprefix $(TI_ARM_MCU_BSP_DIR)/driverlib/,$(BSP_SRCS))
  MK_OBJS+=entry_gcc.o $(BSP_SRCS:%.c=%.o)

  CFLAGS_$(BUILD)/$d/prcm.o+=-Wno-unused-variable
  CFLAGS+=-I$(TI_ARM_MCU_BSP_DIR)/driverlib

  LINKER_SCRIPT:=$(BUILD)/$(d)/cc3200.ld
  CFLAGS+=-mthumb -mcpu=cortex-m4 -O3 -ffunction-sections -fdata-sections \
    -I$(TI_ARM_MCU_BSP_DIR) -Dgcc -fsingle-precision-constant
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(call get_libc_dir) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
