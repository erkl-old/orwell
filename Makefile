AR = ar
CC = gcc
CFLAGS = -O2 -Wall

HEADERS = src/orwell.h                \
          src/orwell-util.h

OBJS = src/orwell.o                   \
       src/orwell-util.o

orwell.a: $(OBJS)
	@printf "  AR    $@\n"
	@$(AR) crs $@ $(OBJS)

src/%.o: src/%.c $(HEADERS)
	@printf "  CC    $@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@printf "  rm    orwell.a\n"
	@printf "  rm    src/*.o\n"
	@rm -f orwell.a
	@rm -f src/*.o

.PHONY: clean
