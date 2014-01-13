MK_OBJS=kl25z.o interrupts.o

LINKER_SCRIPT:=$(BUILD)/$(d)/kl25z.ld
CFLAGS+=-mcpu=cortex-m0 -O0 -gdwarf-2 -mthumb -fomit-frame-pointer -Wstrict-prototypes -fverbose-asm
LDFLAGS+=-nostartfiles --no-warn-mismatch --entry reset_isr
 
LIBS+=-L$(LIBC_DIR) -lm -lc -L$(call get_libgcc_dir) -lgcc
