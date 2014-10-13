MK_OBJS+=x86.o vga_term.o boot.o

ifneq ($(CONFIG_GCC),)
  LINKER_SCRIPT:=$(BUILD)/$(d)/x86.ld
  LDFLAGS+=--entry _start --gc-sections --nostdlib
  LIBS+=-L$(call get_libc_dir) -lm -lc -L$(call get_libgcc_dir) -lgcc
endif
