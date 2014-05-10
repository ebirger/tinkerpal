# Standalone Makefile for the various kconfig executables.
# Based on the Linux Makefile

# We need this, in case the user has it in its environment
#
unexport CONFIG_

obj := ../../$(CONFIG)
src := .
HOSTCC=gcc

# lxdialog stuff
check-lxdialog := ./lxdialog/check-lxdialog.sh

HOST_EXTRACFLAGS += $(shell $(check-lxdialog) -ccflags) -DLOCALE

lxdialog += lxdialog/checklist.o lxdialog/util.o lxdialog/inputbox.o
lxdialog += lxdialog/textbox.o lxdialog/yesno.o lxdialog/menubox.o

conf-objs := $(obj)/conf.o $(obj)/zconf.tab.o
mconf-objs := $(obj)/mconf.o $(obj)/zconf.tab.o $(addprefix $(obj)/,$(lxdialog))

all : $(obj)/conf
menuconfig : $(obj)/mconf

$(obj)/conf : $(conf-objs)
	gcc -o $@ $^

$(obj)/mconf : $(mconf-objs)
	gcc -o $@ $^ $(HOSTLOADLIBES_mconf)

$(obj)/%.o : %.c
	gcc -I. $(HOST_EXTRACFLAGS) -c -o $@ $<

$(obj)/%.o : $(obj)/%.c
	gcc -I. $(HOST_EXTRACFLAGS) -c -o $@ $<

$(obj)/zconf.lex.c : zconf.l
	flex -o$@ -L -P zconf $<

$(obj)/zconf.hash.c : zconf.gperf
	gperf -t --output-file $@ -a -C -E -g -k 1,3,$$ -p -t $<
	
$(obj)/zconf.tab.c : zconf.y
	bison -o$@ -t -l -p zconf $<

# Check that we have the required ncurses stuff installed for lxdialog (menuconfig)
$(addprefix $(obj)/,$(lxdialog)): $(obj)/dochecklxdialog
$(obj)/dochecklxdialog:
	$(Q)$(CONFIG_SHELL) $(check-lxdialog) -check $(HOSTCC) $(HOST_EXTRACFLAGS) $(HOSTLOADLIBES_mconf)

# Add environment specific flags
HOST_EXTRACFLAGS += $(shell $(CONFIG_SHELL) ./check.sh $(HOSTCC) $(HOSTCFLAGS))

ifeq ($(shell uname),Darwin)
  HOSTLOADLIBES_mconf = -lncurses
else
  HOSTLOADLIBES_mconf = $(shell $(CONFIG_SHELL) $(check-lxdialog) -ldflags $(HOSTCC))
endif

$(obj)/zconf.tab.o: $(obj)/zconf.lex.c $(obj)/zconf.hash.c

PHONY += $(obj)/dochecklxdialog menuconfig
