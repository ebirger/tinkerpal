ifneq ($(CONFIG_ENC28J60),)
  MK_OBJS+=enc28j60.o $(if $(CONFIG_JS),js_enc28j60.o)
  MK_DESCS=enc28j60.desc
endif
ifneq ($(CONFIG_LINUX_PACKET_ETH),)
  MK_OBJS+=linux_packet_eth.o $(if $(CONFIG_JS),js_linux_packet_eth.o)
  MK_DESCS=linux_packet_eth.desc
endif
