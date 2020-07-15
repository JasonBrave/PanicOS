OBJS= ld.o
CFLAGS += -I../libc/include -I../libsys/include -fPIE

ld.so : $(OBJS)
	$(LD) -pie --no-dynamic-linker -e ld_main $(OBJS) ../libc.a ../libsys.a -o ld.so

.PHONY: install
install: ld.so
	cp ld.so ..

.PHONY: clean
clean:
	rm -f ld.so $(OBJS)
