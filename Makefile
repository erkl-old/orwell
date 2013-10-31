AR = ar
CC = gcc
CFLAGS = -O2 -Wall

OBJS = src/orwell.o

orwell.a: $(OBJS)
	@printf "  AR    $@\n"
	@$(AR) crs $@ $(OBJS)

src/%.o: src/%.c src/orwell.h
	@printf "  CC    $@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@printf "  rm    orwell.a\n"
	@printf "  rm    src/*.o\n"
	@rm -f orwell.a
	@rm -f src/*.o

.PHONY: clean
