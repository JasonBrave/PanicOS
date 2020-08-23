all:
	$(MAKE) -C hello install
	$(MAKE) -C edu install
	$(MAKE) -C viogpu install
	cp *.mod ../rootfs/boot/module

.PHONY: clean
clean:
	$(MAKE) -C hello clean
	$(MAKE) -C edu clean
	$(MAKE) -C viogpu clean
	rm -f *.mod
