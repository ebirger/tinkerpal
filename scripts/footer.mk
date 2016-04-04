OBJS:=$(OBJS) $(addprefix $(BUILD)/$(d)/,$(MK_OBJS))
JSAPIS:=$(JSAPIS) $(addprefix $(d)/,$(MK_JSAPIS))
BUILTIN_FS_INTERNAL_FILES:=$(BUILTIN_FS_INTERNAL_FILES) $(addprefix $(d)/,$(MK_BUILTIN_FS_INTERNAL_FILES))
ADDITIONAL_INCLUDE_PATHS:=$(ADDITIONAL_INCLUDE_PATHS) $(addprefix $(d)/,$(MK_ADDITIONAL_INCLUDE_PATHS))
ADDITIONAL_INCLUDES:=$(ADDITIONAL_INCLUDES) $(addprefix $(d)/,$(MK_ADDITIONAL_INCLUDES))

$(foreach o,$(MK_OBJS),\
  $(eval MK_CFLAGS_$(BUILD)/$(d)/$(o)=$(MK_CFLAGS_$(o))))

$(foreach m,$(MK_INCLUDES),\
  $(eval include $(addsuffix /$m,$(d))))

$(foreach s,$(MK_SUBDIRS),\
  $(eval $(call include_dir,$s)))

$(foreach l,$(MK_LINKS),\
  $(eval $(call link_target,$(BUILD)/$(d)/$(notdir $l),$l)))
