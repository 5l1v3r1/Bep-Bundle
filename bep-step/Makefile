bep-step: core/src/bep-step.c
	gcc -DLOG_USE_COLOR -g -o bep-step core/src/bep-step.c core/src/log_tree.c core/src/proc.c core/src/log.c core/src/find_new_proc.c -g -Wall -Icore/include
.PHONY: install
install: bep-step
	mkdir -p $(DESDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/bep-step
	mkdir -p /usr/share/man/man1

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/bep-step
.PHONY: clean
clean:
	rm bep-step