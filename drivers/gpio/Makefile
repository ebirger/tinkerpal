MK_OBJS=gpio.o $(if $(CONFIG_JS),js_gpio.o)
MK_JSAPIS=gpio.jsapi

ifeq ($(CONFIG_JS)$(CONFIG_MODULES),yy)
  MK_BUILTIN_FS_INTERNAL_FILES=pin.js
endif
