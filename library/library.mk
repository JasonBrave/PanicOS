CFLAGS += -I../libsys/include -I../libc/include -I..

$(LIB).a : $(OBJS)
	$(AR) -rcs $(LIB).a $(OBJS)

%.o : %.asm
	nasm -felf32 -gdwarf $< -o $@

.PHONY: install
install: $(LIB).a
	cp $(LIB).a ..
	mkdir -p ../../rootfs/devel/include/$(HEADERDIR)
	cp -r $(HEADERS) ../../rootfs/devel/include/$(HEADERDIR)

.PHONY: clean
clean:
	rm -f $(LIB).a $(OBJS)
