define include_dir
  dir_stack := $(d) $(dir_stack)
  d := $(d)/$(1)
  include scripts/header.mk
  include $(addsuffix /Makefile,$$(d))
  include scripts/footer.mk
  d := $$(firstword $$(dir_stack))
  dir_stack := $$(wordlist 2,$$(words $$(dir_stack)),$$(dir_stack))
endef

# $1 target
# $2 source
define link_target

AUTO_GEN_FILES+=$1

$1 :: $2
	@echo "LN $1"
	$(Q)ln -s $2 $1

endef
