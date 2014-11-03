# LM4 Flash Burner

LM4TOOLS=$(STAGING)/lm4tools
LM4TOOLS_FETCHED=$(LM4TOOLS)/.fetched
LM4FLASH_BURNER=$(LM4TOOLS)/lm4flash/lm4flash

$(LM4TOOLS_FETCHED) :
	@echo Fetching LM4Tools
	git clone git://github.com/utzig/lm4tools.git $(LM4TOOLS)
	@touch $(LM4TOOLS_FETCHED)

$(LM4FLASH_BURNER) : $(LM4TOOLS_FETCHED)
	@echo Building LM4Flash
	cd $(LM4TOOLS)/lm4flash; make

# MSPDebug Burner

MSPDEBUG_VER=0.22
MSPDEBUG_URL=http://sourceforge.net/projects/mspdebug/files/mspdebug-$(MSPDEBUG_VER).tar.gz
MSPDEBUG_DIR=$(STAGING)/mspdebug-$(MSPDEBUG_VER)
MSPDEBUG_FETCHED=$(MSPDEBUG_DIR)/.fetched
MSP430_LIB=$(MSPDEBUG_DIR)/libmsp430.so
MSPDEBUG_BURNER=$(MSPDEBUG_DIR)/mspdebug
  
$(MSPDEBUG_FETCHED) :
	@echo Fetching mspdebug
	@cd $(STAGING); wget $(MSPDEBUG_URL) -O - | tar -xzv
	@touch $(MSPDEBUG_FETCHED)

$(MSP430_LIB) :
	@wget www.tinkerpal.org/libmsp430.so -O $(MSP430_LIB)

$(MSPDEBUG_BURNER) : $(MSPDEBUG_FETCHED) $(MSP430_LIB)
	@echo Building mspdebug
	cd $(MSPDEBUG_DIR); make

# STLink Burner

STLINK=$(STAGING)/stlink
STLINK_FETCHED=$(STLINK)/.fetched
STLINK_BURNER=$(STLINK)/st-flash

$(STLINK_FETCHED) :
	@echo Fetching STLink
	git clone git://github.com/texane/stlink.git $(STLINK)
	@touch $(STLINK_FETCHED)

$(STLINK_BURNER) : $(STLINK_FETCHED)
	@echo Building STLink
	cd $(STLINK); ./autogen.sh; ./configure; make

# STM32 Serial Loader

STM32LOADER=$(STAGING)/stm32loader
STM32LOADER_BURNER=$(STM32LOADER)/stm32loader.py

$(STM32LOADER_BURNER) :
	@echo Fetching stm32loader
	git clone git://github.com/jsnyder/stm32loader.git $(STM32LOADER)

# x86 Platform Emulation
X86_BURNER=true && mkdir -p $(BUILD)/isodir/boot && \
  mkdir -p $(BUILD)/isodir/boot/grub && \
  cp $(TARGET) $(BUILD)/isodir/boot/tp.bin && \
  echo "menuentry 'TinkerPal' { multiboot /boot/tp.bin ) }" > $(BUILD)/isodir/boot/grub/grub.cfg && \
  grub-mkrescue -o $(IMAGE:.bin=.iso) $(BUILD)/isodir

# ATMEGA328
ATMEGA328_BURNER=avrdude -c usbtiny -p m328p -Uflash:w:$(TARGET)
