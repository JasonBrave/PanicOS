all:
	$(MAKE) -C hello install
	$(MAKE) -C edu install
	$(MAKE) -C virtgpu install
	cp *.mod ../rootfs/boot/module

.PHONY: clean
clean:
	$(MAKE) -C hello clean
	$(MAKE) -C edu clean
	$(MAKE) -C virtgpu clean
	rm -f *.mod
