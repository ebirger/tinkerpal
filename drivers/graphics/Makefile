MK_OBJS+=$(if $(CONFIG_JS),js_graphics_screens.o)

ifneq ($(CONFIG_ILI93XX),)
  MK_OBJS+=ili93xx.o
  MK_DESCS+=ili93xx.desc
  MK_OBJS+=$(if $(CONFIG_ILI93XX_BITBANG),ili93xx_bitbang.o)
  MK_OBJS+=$(if $(CONFIG_ILI9328),ili9328.o)
  MK_OBJS+=$(if $(CONFIG_ILI9325),ili9325.o)
  MK_OBJS+=$(if $(CONFIG_ILI9320),ili9320.o)
endif

ifneq ($(CONFIG_SDL_SCREEN),)
  MK_OBJS+=sdl_screen.o
  MK_DESCS+=sdl_screen.desc
  CFLAGS+=$(shell sdl-config --cflags)
  LIBS+=$(shell sdl-config --libs)
endif

ifneq ($(CONFIG_DOGS102X6),)
  MK_OBJS+=dogs102x6.o
  MK_DESCS+=dogs102x6.desc
endif

ifneq ($(CONFIG_SSD1306),)
  MK_OBJS+=ssd1306.o
  MK_DESCS+=ssd1306.desc
endif

ifneq ($(CONFIG_SSD1329),)
  MK_OBJS+=ssd1329.o
  MK_DESCS+=ssd1329.desc
endif

ifneq ($(CONFIG_PCD8544),)
  MK_OBJS+=pcd8544.o
  MK_DESCS+=pcd8544.desc
endif

ifneq ($(CONFIG_ST7735),)
  MK_OBJS+=st7735.o
  MK_DESCS+=st7735.desc
endif

ifneq ($(CONFIG_ST7920),)
  MK_OBJS+=st7920.o
endif

MODULES=$(if $(CONFIG_TEXT_LCD),text_lcd.js)
MK_BUILTIN_FS_INTERNAL_FILES=$(MODULES)
