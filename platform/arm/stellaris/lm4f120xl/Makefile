MK_OBJS=lm4f120xl.o interrupts.o

ifeq ($(CONFIG_JS)$(CONFIG_MODULES),yy)
  MK_BUILTIN_FS_INTERNAL_FILES=board_init.js
endif

ifneq ($(CONFIG_TI_CCS5),)
  LINKER_SCRIPT:=$(d)/lm4f120xl.cmd 
  CFLAGS+=-mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me \
  --define=PART_LM4F120H5QR --define=TARGET_IS_BLIZZARD_RA1 \
  --include_path=$(STELLARISWARE_DIR)
  LDFLAGS+=-mv7M4 --abi=eabi -me
  LIBS+=-l"libc.a"
endif

ifneq ($(CONFIG_GCC),)
  MK_OBJS+=entry_gcc.o 

  LINKER_SCRIPT:=$(BUILD)/$(d)/lm4f120xl.ld
  FPU=softfp
  #FPU=hard
  ifeq ($(FPU),hard)
    LIBC_DIR:=$(LIBC_DIR)/float-abi-hard/fpuv4-sp-d16/
    LIBGCC_DIR:=$(LIBGCC_DIR)/float-abi-hard/fpuv4-sp-d16/
  endif
  CFLAGS+=-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=$(FPU) -Os \
    -ffunction-sections -fdata-sections -DPART_LM4F120H5QR \
    -I$(STELLARISWARE_DIR) -DTARGET_IS_BLIZZARD_RA1 -Dgcc -fsingle-precision-constant
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(LIBC_DIR) -lm -lc -L$(LIBGCC_DIR) -lgcc
endif
