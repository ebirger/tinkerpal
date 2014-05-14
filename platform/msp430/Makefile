MK_OBJS=msp430f5529.o msp430f5529_usci.o
MK_OBJS+=$(if $(CONFIG_GPIO),msp430f5529_gpio.o)
MK_OBJS+=$(if $(CONFIG_SPI),msp430f5529_spi.o)

ifneq ($(CONFIG_TI_CCS5),)
  TI_COMPILER:=$(TI_CCS5)/tools/compiler/msp430_4.1.1
  CC=$(TI_COMPILER)/bin/cl430
  LINKER_SCRIPT:=$(d)/lnk_msp430f5529.cmd 
  CFLAGS+=$(if $(POWER_DBG),--advice:power=all)
  CFLAGS+=-vmspx --abi=eabi --define=__MSP430F5529__ \
    --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 \
    --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal \
    --include_path="$(TI_CCS5)/ccs_base/msp430/include"
  LDFLAGS+=-vmspx --abi=eabi -g -i"$(TI_CCS5)/ccs_base/msp430/include"
  LIBS+=-l"libc.a"
endif

ifneq ($(CONFIG_GCC),)
  MSPDEBUG_VER=0.22
  MSPDEBUG_URL=http://sourceforge.net/projects/mspdebug/files/mspdebug-$(MSPDEBUG_VER).tar.gz
  MSPDEBUG_DIR=$(STAGING)/mspdebug-$(MSPDEBUG_VER)
  MSPDEBUG_FETCHED=$(MSPDEBUG_DIR)/.fetched
  BURNER=$(MSPDEBUG_DIR)/mspdebug
  
$(MSPDEBUG_FETCHED) :
	@echo Fetching mspdebug
	@cd $(STAGING); wget $(MSPDEBUG_URL) -O - | tar -xzv
	@touch $(MSPDEBUG_FETCHED)
  
$(BURNER) : $(MSPDEBUG_FETCHED)
	@echo Building mspdebug
	cd $(MSPDEBUG_DIR); make
  
  CFLAGS+=-mmcu=msp430f5529 -mdisable-watchdog -Os
  LDFLAGS+=-mmcu=msp430f5529 -mdisable-watchdog

  BURN_CMD=$(BURNER) tilib "prog $(TARGET)"
endif
