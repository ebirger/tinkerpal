MK_OBJS=msp430f5529.o

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
