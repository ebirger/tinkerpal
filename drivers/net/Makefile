ifneq ($(CONFIG_ENC28J60),)
  MK_OBJS+=enc28j60.o $(if $(CONFIG_JS),js_enc28j60.o)
  MK_JSAPIS=enc28j60.jsapi
endif

ifneq ($(CONFIG_NET_ESP8266),)
  MK_OBJS+=esp8266.o $(if $(CONFIG_JS),js_esp8266.o)
  MK_JSAPIS=esp8266.jsapi
endif
