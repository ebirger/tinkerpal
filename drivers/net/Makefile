ifneq ($(CONFIG_ENC28J60),)
  MK_OBJS+=enc28j60.o $(if $(CONFIG_JS),js_enc28j60.o)
  MK_DESCS=enc28j60.desc
endif

ifneq ($(CONFIG_ESP8266),)
  MK_OBJS+=esp8266.o $(if $(CONFIG_JS),js_esp8266.o)
  MK_DESCS=esp8266.desc
endif
