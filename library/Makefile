all:
	$(MAKE) -C crt
	$(MAKE) -C libsys install
	$(MAKE) -C libc install

.PHONY: clean
clean:
	$(MAKE) -C crt clean
	$(MAKE) -C libsys clean
	$(MAKE) -C libc clean
	rm -f *.a
