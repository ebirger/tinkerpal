# text_to_c - generates a "c" string file from any file and adds it to the 
# build targets.
#
# Usage $(eval $(call text_to_c,$1,$2,$3))
#
# $1 - target
# $2 - source
# $3 - variable name

define text_to_c

AUTO_GEN_FILES+=$1
MK_OBJS+=$(notdir $(1:%.c=%.o))

$1:: $2 scripts/text_to_c_str.sed
	@echo GEN $$@
	$$(Q)echo "#include \"util/tstr.h\"" > $1
	$$(Q)echo -n "tstr_t $3 = S(" >> $1
	$$(Q)cpp -P $2 | sed -f scripts/text_to_c_str.sed  >> $1
	$$(Q)echo ");" >> $1

endef
