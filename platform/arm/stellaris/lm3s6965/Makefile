MK_OBJS+=lm3s6965.o interrupts.o

ifeq ($(CONFIG_JS)$(CONFIG_MODULES),yy)
  MK_BUILTIN_FS_INTERNAL_FILES=board_init.js
endif

ifneq ($(CONFIG_TI_CCS5),)
  CFLAGS+=-mv7M3 --code_state=16 --abi=eabi -me --define=PART_LM3S6965 \
    --include_path=$(STELLARISWARE_DIR)
  LDFLAGS+=-mv7M3 --abi=eabi -me
  LINKER_SCRIPT:=$(d)/lm3s6965.cmd
  # Currently require pre-compiled version of driverlib for ccs5
  LIBS+=-l"libc.a" -l"$(STELLARISWARE_DIR)/driverlib/ccs-cm3/Debug/driverlib-cm3.lib"
endif

ifneq ($(CONFIG_GCC),)
  BSP_SRCS:=systick.c sysctl.c gpio.c interrupt.c uart.c ssi.c cpu.c pwm.c

  MK_LINKS:=$(addprefix $(STELLARISWARE_DIR)/driverlib/,$(BSP_SRCS))

  MK_OBJS+=entry_gcc.o $(BSP_SRCS:%.c=%.o)

  LINKER_SCRIPT:=$(BUILD)/$(d)/lm3s6965.ld
  CFLAGS+=-mthumb -mcpu=cortex-m3 -Os \
    -ffunction-sections -fdata-sections -DPART_LM3S6965 \
    -I$(STELLARISWARE_DIR) -Dgcc -fsingle-precision-constant
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(LIBC_DIR) -lm -lc -L$(LIBGCC_DIR) -lgcc
endif
