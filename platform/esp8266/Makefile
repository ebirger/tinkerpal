ifeq ($(ESP8266_SDK_BASE),)
  $(error Please set ESP8266_SDK_BASE environment variable)
endif

ESPTOOL=esptool.py

MK_OBJS+=esp8266.o

CFLAGS+=-fno-builtin -fno-strict-aliasing -mlongcalls \
  -mtext-section-literals  -D__ets__ -DICACHE_FLASH -Wl,EL \
  -fno-inline-functions -nostdlib

ADDITIONAL_INCLUDE_PATHS+=$(ESP8266_SDK_BASE)/sdk/include \
  $(ESP8266_SDK_BASE)/sdk/examples/driver_lib/include/driver $(d)

LINKER_SCRIPT=$(ESP8266_SDK_BASE)/sdk/ld/eagle.app.v6.ld

LDFLAGS+=-L$(ESP8266_SDK_BASE)/sdk/lib -T$(LINKER_SCRIPT) -u call_user_start \
  -nostdlib -EL

LIBS+=-L$(call get_libc_dir) -lc -L$(call get_libgcc_dir) -lgcc \
  -L$(call get_lib_dir,hal) -lhal -lpp -lphy -lnet80211 -llwip -lwpa -lmain

IMAGE_NAME:=tp-0x00000.bin tp-0x40000.bin

INTRM_TARGET=$(BUILD)/interim.o

AUTO_GEN_FILES+=$(INTRM_TARGET)

# Linkage trickery:
#   - Would like to use the SDK provided linker script
#   - The SDK linker script places .text in iram1_0 seg which is quite
#     small (e.g. 32KB)
#   - So place the .text section in irom_0_0_seg by renaming it to
#     .irom0.text after partial linking
define target_link
  @echo LD $@
  $(Q)$(LD) --no-check-sections -static -r -o $(INTRM_TARGET) \
    $(OBJS)
  $(Q)$(OBJCOPY) --rename-section .text=.irom0.text $(INTRM_TARGET)
  $(Q)$(LD) $(LDFLAGS) -o $@ $(INTRM_TARGET) --start-group $(LIBS) --end-group
  $(Q)rm $(INTRM_TARGET)
endef

gen_image=$(ESPTOOL) elf2image $^
