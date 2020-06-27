fs.img:
	../tools/xv6fs ../fs.img init sh dir

rootfs.cpio:
	find . -print -depth | cpio -ov > ../rootfs.cpio

.PHONY: clean
clean:
	find . -depth -not -name "Makefile" -delete
