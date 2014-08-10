$(info ==== Using TI CCS5 environment ====)
TI_CCS5?=$(CONFIG_TI_CCS5_DIR:"%"=%)

CFLAGS+=--preinclude $(BUILD)/autoconf.h
CFLAGS+=-O2 -g --gcc --define=ccs="ccs" --diag_warning=225 \
  --display_error_number --gen_func_subsections=on \
  --preproc_with_compile 

INC+=--include_path=./ --include_path=$(BUILD) \
  --include_path="$(TI_COMPILER)/include" \
  $(addprefix --preinclude,$(ADDITIONAL_INCLUDES)) \
  $(addprefix --include_path=,$(ADDITIONAL_INCLUDE_PATHS))

# These flags must come after the regular ldflags (specifically -z)
ADDITIONAL_LDFLAGS=-O2 -g --gcc --define=ccs="ccs" --diag_warning=225 \
  --display_error_number --gen_func_subsections=on -z \
  -m"$(BUILD)/tp.map" -i"$(TI_COMPILER)/lib" -i"$(TI_COMPILER)/include" \
  --reread_libs --warn_sections --display_error_number --rom_model

# Build commands
compile=$(CC) $(CFLAGS) $(MK_CFLAGS_$@) $(INC) --output_file $@ $<
link=$(CC) $(LDFLAGS) $(ADDITIONAL_LDFLAGS) -i=$(BUILD) --output_file $@ $(OBJS) $(LIBS) $(LINKER_SCRIPT)
obj_to_bin=

# Dependencies - alas, no automatic header dependency generation yet, 
# have all objs rely on all headers
calc_deps=
include_deps=$(eval $(OBJS) : $(shell find . -name "*.h"))

# Targets
TARGET_NAME:=tp.out
IMAGE_NAME:=tp.bin
