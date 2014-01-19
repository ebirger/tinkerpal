# $1 source file path
define file_id
$(basename $(notdir $1))
endef

# $1 source file path
define file_name
builtin_file_$(call file_id,$1)
endef

# $1 source file path
define file_str_file
$(BUILD)/$(d)/$(call file_name,$1).c
endef

# $1 source file path
define file_path
$(shell echo $1 | sed "s/^\([\"']\)\(.*\)\1\$$/\2/g")
endef

# $1 source file path
define file_desc
{ .name = \"$(call file_id,$1)\", .content = &$(call file_name,$1), }
endef

FILES+=$(BUILTIN_FS_INTERNAL_FILES)
# External files
ifneq ($(CONFIG_BUILTIN_FS_EXTERNAL_PATH),"")
  FILES+=$(wildcard $(CONFIG_BUILTIN_FS_EXTERNAL_PATH:"%"=%)/*.*)
endif

# Convert files into objects
$(foreach f,$(FILES),\
  $(eval $(call text_to_c,$(call file_str_file,$f),$(call file_path,$f),\
    $(call file_name,$f))))

# Genrate builtin_fs_files.c
FS_TARGET:=builtin_fs_files.c
FS_FILE:=$(BUILD)/$(d)/$(FS_TARGET)
AUTO_GEN_FILES+=$(FS_FILE)
MK_OBJS+=builtin_fs.o $(FS_TARGET:%.c=%.o)

$(FS_FILE): $(FILES) $(BUILD)/auto.conf
	@echo GEN $@
	$(Q)echo "/* Automatically generated file, DO NOT MANUALLY EDIT */" > $@
	$(Q)echo "#include \"drivers/fs/builtin_fs/builtin_fs.h\"" >> $@
	$(Q)$(foreach f,$(FILES),printf "extern char *$(call file_name,$f);\n" >> $@;)
	$(Q)echo "const builtin_fs_file_t builtin_fs_files[] = {" >> $@
	$(Q)$(foreach f,$(FILES),printf "$(call file_desc,$f),\n" >> $@;)
	$(Q)echo "{}" >> $@
	$(Q)echo "};" >> $@
