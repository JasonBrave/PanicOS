rootfs.cpio:
	find . -print -depth | cpio -ov > ../rootfs.cpio

.PHONY: clean
clean:
	find . -depth -not -name "Makefile" -delete
