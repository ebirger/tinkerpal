all: docs

export DOCS_DIR:=$(BUILD)/doc

DESCS=$(shell find . -name "*.desc")

RESTDOWN_DIR=$(STAGING)/restdown
RESTDOWN_EXEC=$(RESTDOWN_DIR)/bin/restdown
RESTDOWN=python $(RESTDOWN_EXEC)
$(RESTDOWN_DIR)/.git:
	@git clone https://github.com/trentm/restdown.git $(RESTDOWN_DIR)

$(RESTDOWN_EXEC): | $(RESTDOWN_DIR)/.git

$(DOCS_DIR)/doc.h: $(DESCS)
	@mkdir -p $(DOCS_DIR)
	@cat $^ > $@

$(DOCS_DIR)/tdocs: doc/tdocs.c $(DOCS_DIR)/doc.h doc/tdocs.h $(BUILD)/autoconf.h
	@gcc -I. -I$(BUILD) -include $(BUILD)/autoconf.h -o $@ $<

$(DOCS_DIR)/manual.html: doc/manual.txt
	@asciidoc -b html5 -o $@ $<

$(DOCS_DIR)/api.restdown: $(BUILD)/doc/tdocs
	@$< $@

$(DOCS_DIR)/api.html: $(DOCS_DIR)/api.restdown | $(RESTDOWN_EXEC)
	$(RESTDOWN) $(RESTDOWN_FLAGS) -m $(DOCS_DIR) $<

docs: $(DOCS_DIR)/api.html $(DOCS_DIR)/manual.html

.PHONY: docs
