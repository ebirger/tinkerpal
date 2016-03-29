all: docs

export DOCS_DIR:=$(BUILD)/doc

DESCS=$(shell find . -name "*.desc")
CHIPS=$(shell find . -name "*.chip")

RESTDOWN_DIR=$(STAGING)/restdown
RESTDOWN_EXEC=$(RESTDOWN_DIR)/bin/restdown
RESTDOWN=python2 $(RESTDOWN_EXEC)
$(RESTDOWN_DIR)/.git:
	@git clone https://github.com/trentm/restdown.git $(RESTDOWN_DIR)

$(RESTDOWN_EXEC): | $(RESTDOWN_DIR)/.git

$(DOCS_DIR)/chips.h: $(CHIPS)
	@cat $^ > $@

$(DOCS_DIR)/doc.h: $(DESCS)
	@mkdir -p $(DOCS_DIR)
	@cat $^ > $@

$(DOCS_DIR)/gen_js_api: doc/gen_js_api.c $(DOCS_DIR)/doc.h doc/gen_js_api.h $(BUILD)/autoconf.h
	@gcc -I. -I$(BUILD) -include $(BUILD)/autoconf.h -o $@ $<

$(DOCS_DIR)/gen_pin_mappings: doc/gen_pin_mappings.c $(DOCS_DIR)/chips.h $(BUILD)/autoconf.h
	@gcc -I. -I$(BUILD) -include $(BUILD)/autoconf.h -o $@ $<

$(DOCS_DIR)/manual.html: doc/manual.txt
	@asciidoc -b html5 -o $@ $<

$(DOCS_DIR)/api.restdown: $(BUILD)/doc/gen_js_api
	@$< $@

$(DOCS_DIR)/%.html: $(DOCS_DIR)/%.restdown | $(RESTDOWN_EXEC)
	$(RESTDOWN) $(RESTDOWN_FLAGS) -m $(DOCS_DIR) $<

$(DOCS_DIR)/%.rst: $(DOCS_DIR)/%.restdown
	@pandoc --from=markdown --to=rst --output=$@ $<

$(DOCS_DIR)/pin_mappings.restdown: $(DOCS_DIR)/gen_pin_mappings
	@$< $@

docs: $(DOCS_DIR)/api.html $(DOCS_DIR)/manual.html $(DOCS_DIR)/api.rst $(DOCS_DIR)/pin_mappings.html $(DOCS_DIR)/pin_mappings.rst

.PHONY: docs
