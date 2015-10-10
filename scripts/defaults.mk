define target_link_default
	@echo $($(quiet_)link)
	@$(call link)
endef

define target_link
	@$(call target_link_default)
endef
