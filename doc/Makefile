all: docs

export DOCS_DIR:=$(BUILD)/doc

DESCS=$(shell find . -name "*.desc")

$(DOCS_DIR)/doc.h: $(DESCS)
	@mkdir -p $(DOCS_DIR)
	@cat $^ > $@

$(DOCS_DIR)/tdocs: doc/tdocs.c $(DOCS_DIR)/doc.h doc/tdocs.h $(BUILD)/autoconf.h
	@gcc -I. -I$(BUILD) -include $(BUILD)/autoconf.h -o $@ $<

$(DOCS_DIR)/manual.html: doc/manual.txt
	@asciidoc -b html5 -o $@ $<

docs: $(BUILD)/doc/tdocs $(DOCS_DIR)/manual.html
	@$<

.PHONY: docs
