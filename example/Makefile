AR = ar
CC = gcc
CFLAGS = -O2 -Wall

example: example.c ../orwell.a
	@printf "  CC    $@\n"
	@$(CC) $(CFLAGS) -o $@ example.c ../orwell.a

../orwell.a:
	make -C .. orwell.a

clean:
	@printf "  rm    example\n"
	@rm -f example

.PHONY:
	clean
