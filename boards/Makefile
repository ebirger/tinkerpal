MK_OBJS+=$(if $(CONFIG_RDK_IDM),board_rdk_idm.o)
MK_OBJS+=$(if $(CONFIG_EK_LM3S6965),board_ek_lm3s6965.o)
MK_OBJS+=$(if $(CONFIG_EK_LM4F120XL),board_ek_lm4f120xl.o)
MK_OBJS+=$(if $(CONFIG_EK_TM4C123GXL),board_ek_tm4c123gxl.o)
MK_OBJS+=$(if $(CONFIG_EK_TM4C1294XL),board_ek_tm4c1294xl.o)
MK_OBJS+=$(if $(CONFIG_CC3200),board_cc3200_launchxl.o)
MK_OBJS+=$(if $(CONFIG_STM32F3DISCOVERY),board_stm32f3discovery.o)
MK_OBJS+=$(if $(CONFIG_STM32F4DISCOVERY),board_stm32f4discovery.o)
MK_OBJS+=$(if $(CONFIG_STM32F429IDISCOVERY),board_stm32f429idiscovery.o)
MK_OBJS+=$(if $(CONFIG_STM32_ARMJISHU_28),board_stm32_armjishu_28.o)
MK_OBJS+=$(if $(CONFIG_STM32_HY_24),board_stm32_hy_24.o)
MK_OBJS+=$(if $(CONFIG_FRDM_KL25Z),board_frdm_kl25z.o)
MK_OBJS+=$(if $(CONFIG_MSP430F5529_EXP),board_msp430f5529.o)
MK_OBJS+=$(if $(CONFIG_MSP430F5529_LAUNCHPAD),board_msp430f5529.o)
MK_OBJS+=$(if $(CONFIG_PLATFORM_EMULATION),board_unix_sim.o)
MK_OBJS+=$(if $(CONFIG_X86_PLATFORM_EMULATION),board_x86_sim.o)
MK_OBJS+=$(if $(CONFIG_TRINKET_PRO),board_trinket_pro.o)
MK_OBJS+=$(if $(CONFIG_ESP8266_512KB),board_esp8266.o)

include boards/burners.mk

ifneq ($(CONFIG_EK_LM4F120XL)$(CONFIG_EK_TM4C123GXL)$(CONFIG_EK_TM4C1294XL),)
  BURNER=$(LM4FLASH_BURNER)
  BURN_CMD=$(BURNER) $(IMAGE)
endif

ifneq ($(CONFIG_MSP430F5529_LAUNCHPAD),)
  BURNER=$(MSPDEBUG_BURNER)
  BURN_CMD=LD_LIBRARY_PATH=$(MSPDEBUG_DIR) $(BURNER) tilib "prog $(TARGET)"
endif

ifneq ($(CONFIG_STM32F3DISCOVERY)$(CONFIG_STM32F4DISCOVERY)$(CONFIG_STM32F429IDISCOVERY),)
  BURNER=$(STLINK_BURNER)
  BURN_CMD=$(BURNER) write $(IMAGE) 0x08000000
endif

ifneq ($(CONFIG_STM32_HY_24),)
  PORT?=/dev/ttyUSB1
  BURNER=$(STM32LOADER_BURNER)
  BURN_NOTE='You can set the port by running PORT=<your port> make burn'
  BURN_CMD=python $(BURNER) -p $(PORT) -evw $(IMAGE)
endif

ifneq ($(CONFIG_X86_PLATFORM_EMULATION),)
  BURN_CMD=$(X86_BURNER)
endif

ifneq ($(CONFIG_TRINKET_PRO),)
  BURN_CMD=$(ATMEGA328_BURNER)
endif

# Simulation
ifneq ($(CONFIG_EK_LM3S6965),)
  SIMULATE_NOTE='Use CTRL-a-X to exit'
  SIMULATE_CMD=qemu-system-arm -s -M lm3s6965evb -nographic -semihosting -no-reboot -net none -kernel $(IMAGE)
endif

ifneq ($(CONFIG_X86_PLATFORM_EMULATION),)
  SIMULATE_CMD=qemu-system-i386 -cdrom $(IMAGE:.bin=.iso)
endif

ifneq ($(CONFIG_TRINKET_PRO),)
  SIMAVR=$(STAGING)/simavr
  SIMAVR_FETCHED=$(SIMAVR)/.fetched
  AVR_SIMULATOR=$(SIMAVR)/simavr/run_avr
  SIMULATOR=$(AVR_SIMULATOR)
  SIMULATE_CMD=$(AVR_SIMULATOR) -g -vv -m atmega328p -f 16000000 $(TARGET)

$(SIMAVR_FETCHED) :
	@echo "Fetching simavr"
	git clone git://gitorious.org/simavr/simavr.git $(SIMAVR)
	@touch $(SIMAVR_FETCHED)

$(AVR_SIMULATOR) : $(SIMAVR_FETCHED)
	@cd $(SIMAVR) && CC=gcc make all && cd -
endif
