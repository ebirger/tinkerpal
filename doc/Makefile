all: docs

export DOCS_DIR:=$(BUILD)/doc

DESCS=$(shell find . -name "*.desc")

RESTDOWN_DIR=$(STAGING)/restdown
RESTDOWN_EXEC=$(RESTDOWN_DIR)/bin/restdown
RESTDOWN=python2 $(RESTDOWN_EXEC)
$(RESTDOWN_DIR)/.git:
	@git clone https://github.com/trentm/restdown.git $(RESTDOWN_DIR)

$(RESTDOWN_EXEC): | $(RESTDOWN_DIR)/.git

$(DOCS_DIR)/doc.h: $(DESCS)
	@mkdir -p $(DOCS_DIR)
	@cat $^ > $@

$(DOCS_DIR)/gen_js_api: doc/gen_js_api.c $(DOCS_DIR)/doc.h doc/gen_js_api.h $(BUILD)/autoconf.h
	@gcc -I. -I$(BUILD) -include $(BUILD)/autoconf.h -o $@ $<

$(DOCS_DIR)/manual.html: doc/manual.txt
	@asciidoc -b html5 -o $@ $<

$(DOCS_DIR)/api.restdown: $(BUILD)/doc/gen_js_api
	@$< $@

$(DOCS_DIR)/api.html: $(DOCS_DIR)/api.restdown | $(RESTDOWN_EXEC)
	$(RESTDOWN) $(RESTDOWN_FLAGS) -m $(DOCS_DIR) $<

docs: $(DOCS_DIR)/api.html $(DOCS_DIR)/manual.html

.PHONY: docs
