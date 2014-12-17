ifneq ($(CONFIG_ATMEGA328),)
  MK_OBJS+=atmega328.o
  MK_OBJS+=$(if $(CONFIG_I2C),atmega328_i2c.o)
  CFLAGS+=-mmcu=atmega328p -Os --function-sections
  LDFLAGS+=-mmcu=atmega328p -Wl,--gc-sections
endif
