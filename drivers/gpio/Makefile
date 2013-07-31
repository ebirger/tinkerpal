MK_OBJS=gpio.o $(if $(CONFIG_JS),js_gpio.o)
MK_DESCS=gpio.desc

ifeq ($(CONFIG_JS)$(CONFIG_MODULES),yy)
  MK_BUILTIN_FS_INTERNAL_FILES=pin.js
endif
