OBJS = printf.o ulib.o umalloc.o usys.o
CFLAGS += -Iinclude
ASFLAGS += -Iinclude

lib.a: $(OBJS)
	$(AR) -rcs lib.a $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) lib.a
