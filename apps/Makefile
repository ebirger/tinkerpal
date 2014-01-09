ifneq ($(CONFIG_APP_STATIC_FILE),)

  MK_OBJS+=static_file.o

  STATIC_FILE_STR:=$(BUILD)/$(d)/static_file_string.c
  STATIC_FILE_PATH=$(shell echo $(CONFIG_APP_STATIC_FILE_PATH) |\
    sed "s/^\([\"']\)\(.*\)\1\$$/\2/g")

  $(eval $(call text_to_c,$(STATIC_FILE_STR),$(STATIC_FILE_PATH),static_file))

endif

MK_OBJS+=$(if $(CONFIG_APP_FILE_LOADER),file_loader.o)
MK_OBJS+=$(if $(CONFIG_CLI),cli.o history.o)
MK_OBJS+=$(if $(CONFIG_APP_JSCONSOLE),jsconsole.o)
MK_OBJS+=$(if $(CONFIG_APP_ECHO_CONSOLE),echo_console.o)
MK_OBJS+=$(if $(CONFIG_APP_NET_TEST),net_test.o)
MK_OBJS+=$(if $(CONFIG_APP_GRAPHICS_TEST),graphics_test.o)
MK_OBJS+=$(if $(CONFIG_APP_DUMMY),dummy.o)
