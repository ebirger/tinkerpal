MK_OBJS+=$(if $(CONFIG_PLATFORM_EMULATION),sim.o)
MK_OBJS+=$(if $(CONFIG_UNIX),unix.o)

ifneq ($(CONFIG_NETIF_INET),)
  MK_OBJS+=netif_inet.o $(if $(CONFIG_JS),js_netif_inet.o)
  MK_JSAPIS=netif_inet.jsapi
endif

ifneq ($(CONFIG_LINUX_ETH),)
  MK_OBJS+=linux_eth.o $(if $(CONFIG_JS),js_linux_eth.o)
  MK_JSAPIS=linux_eth.jsapi
endif

LIBS+=-lm
