MK_OBJS=main.o
ifneq ($(CONFIG_PLAT_HAS_SERIAL),)
  MK_OBJS+=console.o $(if $(CONFIG_JS),js_console.o)
  MK_DESCS=console.desc
endif
