ifneq ($(CONFIG_ENC28J60),)
  MK_OBJS+=enc28j60.o $(if $(CONFIG_JS),js_enc28j60.o)
  MK_DESCS=enc28j60.desc
endif
MK_OBJS+=$(if $(CONFIG_LINUX_PACKET_ETH),linux_packet_eth.o)
