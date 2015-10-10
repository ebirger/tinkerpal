ifneq ($(CONFIG_STELLARIS),)
  TI_ARM_MCU_BSP_DIR=$(BSPS_DIR)/stellarisware
endif
ifneq ($(CONFIG_TIVA_C),)
  TI_ARM_MCU_BSP_DIR=$(BSPS_DIR)/tivaware
endif
ifneq ($(CONFIG_CC3200),)
  TI_ARM_MCU_BSP_DIR=$(BSPS_DIR)/cc3200-sdk
endif

ADDITIONAL_INCLUDES:=$(ADDITIONAL_INCLUDES) $(d)/ti_arm_mcu_bsp.h
MK_OBJS=ti_arm_mcu.o 
MK_OBJS+=$(if $(CONFIG_TI_CCS5),stellaris_entry_ccs5.o)
MK_OBJS+=$(if $(CONFIG_GPIO),ti_arm_mcu_gpio.o)
MK_OBJS+=$(if $(CONFIG_SPI),ti_arm_mcu_spi.o)
MK_OBJS+=$(if $(CONFIG_I2C),ti_arm_mcu_i2c.o)
MK_OBJS+=$(if $(CONFIG_USB),ti_arm_mcu_usb.o)
ifneq ($(CONFIG_STELLARIS_ETH),)
  MK_OBJS+=stellaris_eth.o $(if $(CONFIG_JS),js_stellaris_eth.o)
  MK_DESCS+=stellaris_eth.desc
endif
MK_OBJS+=$(if $(CONFIG_TIVA_C_EMAC),tiva_c_emac.o)

MK_SUBDIRS+=$(if $(CONFIG_LM4F120XL),lm4f120xl)
MK_SUBDIRS+=$(if $(CONFIG_LM3S6965),lm3s6965)
MK_SUBDIRS+=$(if $(CONFIG_LM3S6918),lm3s6918)
MK_SUBDIRS+=$(if $(CONFIG_TM4C123G),tm4c123g)
MK_SUBDIRS+=$(if $(CONFIG_TM4C1294),tm4c1294)
MK_SUBDIRS+=$(if $(CONFIG_CC3200),cc3200)

ifneq ($(CONFIG_TI_CCS5),)
  TI_COMPILER:=$(TI_CCS5)/tools/compiler/tms470_4.9.5
  CC=$(TI_COMPILER)/bin/cl470
  TI_OBJ2BIN_PATH:=$(TI_CCS5)/utils/tiobj2bin
  OBJ2BIN=$(shell cygpath -u "$(TI_OBJ2BIN_PATH)/tiobj2bin.bat")

  gen_image=$(OBJ2BIN) $< $(IMAGE) "$(TI_COMPILER)/bin/ofd470" \
    "$(TI_COMPILER)/bin/hex470" "$(TI_OBJ2BIN_PATH)/mkhex4bin"
  CFLAGS+=--abi=eabi -me --ual --include_path=$(TI_ARM_MCU_BSP_DIR)
  LDFLAGS+=--abi=eabi -me --ual
  LIBS+=-l"libc.a"
endif

ifneq ($(CONFIG_GCC),)
  ifeq ($(CROSS_COMPILE),)
    $(error Please set CROSS_COMPILE environment variable)
  endif
endif
