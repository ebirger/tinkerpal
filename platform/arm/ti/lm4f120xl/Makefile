MK_OBJS=lm4f120xl.o interrupts.o

ifneq ($(CONFIG_TI_CCS5),)
  LINKER_SCRIPT:=$(d)/lm4f120xl.cmd 
  CFLAGS+=-mv7M4 --code_state=16 --float_support=FPv4SPD16 \
    --define=PART_LM4F120H5QR --define=TARGET_IS_BLIZZARD_RA1
  LDFLAGS+=-mv7M4
endif

ifneq ($(CONFIG_GCC),)
  MK_OBJS+=entry_gcc.o 

  LINKER_SCRIPT:=$(BUILD)/$(d)/lm4f120xl.ld
  CFLAGS+=-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -O3 \
    -ffunction-sections -fdata-sections -DPART_LM4F120H5QR \
    -I$(STELLARISWARE_DIR) -DTARGET_IS_BLIZZARD_RA1 -Dgcc -fsingle-precision-constant
  LDFLAGS+=--entry reset_isr --gc-sections --nostdlib
  LIBS+=-L$(call get_libc_dir) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
