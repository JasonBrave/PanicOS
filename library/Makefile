all:
	$(MAKE) -C crt
	$(MAKE) -C libsys install
	$(MAKE) -C libc install
	$(MAKE) -C libcpu install
	cp *.so ../rootfs/lib
	cp *.a ../rootfs/devel/lib
	cp crt/*.o ../rootfs/devel/lib

.PHONY: clean
clean:
	$(MAKE) -C crt clean
	$(MAKE) -C libsys clean
	$(MAKE) -C libc clean
	$(MAKE) -C libcpu clean
	rm -f *.a *.so
