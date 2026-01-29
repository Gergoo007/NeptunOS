MODULES := $(shell find . -maxdepth 1 -mindepth 1 -type d -not -name template -printf '%P\n')

modules:
	@for mod in $(MODULES); do \
		echo "  > [Mod] $$mod"; \
		$(MAKE) --no-print-directory -C $$mod module || exit 1; \
	done
	@tar -cf modules.tar -C template/out $(foreach MODNAME,$(MODULES),-C ../../$(MODNAME)/out/ $(MODNAME).ko)
	@cp modules.tar ../../boot/

clean:
	@for mod in $(MODULES); do \
		$(MAKE) --no-print-directory -C $$mod clean || exit 1; \
	done
