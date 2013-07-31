$(info ==== Using TI Stellaris CCS environment ====)
TI_CCS5?=$(CONFIG_TI_CCS5_DIR:"%"=%)
TI_COMPILER:=$(TI_CCS5)/tools/compiler/tms470_4.9.5
TI_OBJ2BIN_PATH:=$(TI_CCS5)/utils/tiobj2bin
CC=$(TI_COMPILER)/bin/cl470
OBJ2BIN=$(shell cygpath -u "$(TI_OBJ2BIN_PATH)/tiobj2bin.bat")

CFLAGS+=--preinclude $(BUILD)/autoconf.h
CFLAGS+=--code_state=16 -O2 -g --gcc --define=ccs="ccs" 
  --diag_warning=225 --display_error_number \
  --gen_func_subsections=on --ual --preproc_with_compile 

INC+=--include_path=./ --include_path=$(BUILD) \
  --include_path="$(TI_COMPILER)/include"

# These flags must come after the regular ldflags (specifically -z)
ADDITIONAL_LDFLAGS=-O2 -g --gcc --define=ccs="ccs" --diag_warning=225 \
  --display_error_number --gen_func_subsections=on --ual -z \
  -m"$(BUILD)/tp.map" -i"$(TI_COMPILER)/lib" -i"$(TI_COMPILER)/include" \
  --reread_libs --warn_sections --display_error_number --rom_model

# Build commands
compile=$(CC) $(CFLAGS) $(MK_CFLAGS_$@) $(INC) --output_file $@ $<
link=$(CC) $(LDFLAGS) $(ADDITIONAL_LDFLAGS) -i=$(BUILD) --output_file $@ $(OBJS) $(LIBS) $(LINKER_SCRIPT)
obj_to_bin=$(OBJ2BIN) $< $@ "$(TI_COMPILER)/bin/ofd470" \
  "$(TI_COMPILER)/bin/hex470" "$(TI_OBJ2BIN_PATH)/mkhex4bin"

# Dependencies - alas, no automatic header dependency generation yet, 
# have all objs rely on all headers
calc_deps=
include_deps=$(eval $(OBJS) : $(shell find . -name "*.h"))

# Targets
TARGET_NAME:=tp.out
IMAGE_NAME:=tp.bin
