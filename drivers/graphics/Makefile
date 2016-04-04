MK_OBJS+=$(if $(CONFIG_JS),js_graphics_screens.o)

ifneq ($(CONFIG_ILI93XX),)
  MK_OBJS+=ili93xx.o
  MK_JSAPIS+=ili93xx.jsapi
  MK_OBJS+=$(if $(CONFIG_ILI93XX_BITBANG),ili93xx_bitbang.o)
  MK_OBJS+=$(if $(CONFIG_ILI9328),ili9328.o)
  MK_OBJS+=$(if $(CONFIG_ILI9325),ili9325.o)
  MK_OBJS+=$(if $(CONFIG_ILI9320),ili9320.o)
endif

ifneq ($(CONFIG_SDL_SCREEN),)
  MK_OBJS+=sdl_screen.o
  MK_JSAPIS+=sdl_screen.jsapi
  CFLAGS+=$(shell sdl-config --cflags)
  LIBS+=$(shell sdl-config --libs)
endif

ifneq ($(CONFIG_DOGS102X6),)
  MK_OBJS+=dogs102x6.o
  MK_JSAPIS+=dogs102x6.jsapi
endif

ifneq ($(CONFIG_SSD1306),)
  MK_OBJS+=ssd1306.o
  MK_JSAPIS+=ssd1306.jsapi
endif

ifneq ($(CONFIG_SSD1329),)
  MK_OBJS+=ssd1329.o
  MK_JSAPIS+=ssd1329.jsapi
endif

ifneq ($(CONFIG_PCD8544),)
  MK_OBJS+=pcd8544.o
  MK_JSAPIS+=pcd8544.jsapi
endif

ifneq ($(CONFIG_ST7735),)
  MK_OBJS+=st7735.o
  MK_JSAPIS+=st7735.jsapi
endif

ifneq ($(CONFIG_ST7920),)
  MK_OBJS+=st7920.o
  MK_JSAPIS+=st7920.jsapi
endif

ifneq ($(CONFIG_DUMMY_CANVAS),)
  MK_OBJS+=dummy_canvas.o
  MK_JSAPIS+=dummy_canvas.jsapi
endif

MODULES=$(if $(CONFIG_TEXT_LCD),text_lcd.js)
MK_BUILTIN_FS_INTERNAL_FILES=$(MODULES)
