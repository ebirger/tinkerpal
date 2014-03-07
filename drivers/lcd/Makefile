ifneq ($(CONFIG_ILI93XX),)
  MK_OBJS+=ili93xx.o
  MK_DESCS=ili93xx.desc
  MK_OBJS+=$(if $(CONFIG_JS),js_ili93xx.o)
  MK_OBJS+=$(if $(CONFIG_ILI9328),ili9328.o)
  MK_OBJS+=$(if $(CONFIG_ILI9325),ili9325.o)
endif

ifneq ($(CONFIG_SDL_SCREEN),)
  MK_OBJS+=sdl_screen.o
  MK_OBJS+=$(if $(CONFIG_JS),js_sdl_screen.o)
  MK_DESCS=sdl_screen.desc
  CFLAGS+=$(shell sdl-config --cflags)
  LIBS+=$(shell sdl-config --libs)
endif

ifneq ($(CONFIG_DOGS102X6),)
  MK_OBJS+=dogs102x6.o
  MK_DESCS=dogs102x6.desc
  MK_OBJS+=$(if $(CONFIG_JS),js_dogs102x6.o)
endif

MODULES=$(if $(CONFIG_TEXT_LCD),text_lcd.js)
MK_BUILTIN_FS_INTERNAL_FILES=$(MODULES)
