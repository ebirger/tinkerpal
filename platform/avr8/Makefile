ifneq ($(CONFIG_ATMEGA328),)
  MK_OBJS+=atmega328.o
  CFLAGS+=-mmcu=atmega328p -Os --function-sections
  LDFLAGS+=-mmcu=atmega328p -Wl,--gc-sections
endif
