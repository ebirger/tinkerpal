OBJS:=$(OBJS) $(addprefix $(BUILD)/$(d)/,$(MK_OBJS))
DESCS:=$(DESCS) $(addprefix $(d)/,$(MK_DESCS))
BUILTIN_FS_INTERNAL_FILES:=$(BUILTIN_FS_INTERNAL_FILES) $(addprefix $(d)/,$(MK_BUILTIN_FS_INTERNAL_FILES))

$(foreach o,$(MK_OBJS),\
  $(eval MK_CFLAGS_$(BUILD)/$(d)/$(o)=$(MK_CFLAGS_$(o))))

$(foreach m,$(MK_INCLUDES),\
  $(eval include $(addsuffix /$m,$(d))))

$(foreach s,$(MK_SUBDIRS),\
  $(eval $(call include_dir,$s)))

$(foreach l,$(MK_LINKS),\
  $(eval $(call link_target,$(BUILD)/$(d)/$(notdir $l),$l)))
