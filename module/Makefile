all:
	$(MAKE) -C hello install
	$(MAKE) -C edu install
	cp *.mod ../rootfs/boot/module

.PHONY: clean
clean:
	$(MAKE) -C hello clean
	$(MAKE) -C edu clean
	rm -f *.mod
