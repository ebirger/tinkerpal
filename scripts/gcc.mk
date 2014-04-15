CROSS_COMPILE?=$(CONFIG_CROSS_COMPILE:"%"=%)

$(info ==== Using $(if $(CROSS_COMPILE),Cross,Local) GCC environment ====)

CC=$(CROSS_COMPILE)gcc
# Note: we use gcc as ld on Unix builds so that we get crtX.o paths
LD=$(if $(CONFIG_UNIX),$(CC),$(CROSS_COMPILE)ld)
OBJCOPY=$(CROSS_COMPILE)objcopy
#LDFLAGS+=$(if $(CONFIG_UNIX),-Wl,)-Map=$@.map
LINK_DEPS=$(LINKER_SCRIPT)

CFLAGS+=-I. -I$(BUILD) -include $(BUILD)/autoconf.h -Wall -Werror -g -ansi \
  -std=gnu99
get_libgcc_dir=$(shell dirname $(shell $(CC) $(CFLAGS) -print-libgcc-file-name))
get_libc_dir=$(shell dirname $(shell $(CC) $(CFLAGS) -print-file-name=libc.a))

# Build commands
compile=$(CC) $(CFLAGS) $(MK_CFLAGS_$@) -c -o $@ $<
calc_deps=$(CC) $(CFLAGS) -MM  -MT $@ -MF $(@:.o=.d) $<
include_deps=$(eval -include $(OBJS:.o=.d)) \
  $(if $(LINKER_SCRIPT),$(eval -include $(LINKER_SCRIPT:.ld=.ld.d)))
link=$(LD) $(LDFLAGS) $(if $(LINKER_SCRIPT),-T $(LINKER_SCRIPT)) -o $@ $(OBJS) $(LIBS)
obj_to_bin=$(OBJCOPY) -O binary $< $@

# Targets
TARGET_NAME:=tp
IMAGE_NAME:=tp.bin

# Rules
$(BUILD)/%.ld : %.ld.S
	@echo GEN $@
	$(Q)cpp -MM -MT $@ -MF $(@:.ld=.ld.d) -I. -P $< $@
	$(Q)cpp -I. -P $< $@
