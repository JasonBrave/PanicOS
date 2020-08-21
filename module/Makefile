all:
	$(MAKE) -C hello install
	cp *.mod ../rootfs/boot/module

.PHONY: clean
clean:
	$(MAKE) -C hello clean
	rm -f *.mod
