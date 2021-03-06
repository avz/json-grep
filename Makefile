CC?=cc
LD=$(CC)

PROJECT=json-grep

OBJS=main.o parser.o filter.o
VPATH=src

CFLAGS?=-O2

build: $(PROJECT)

$(PROJECT): $(OBJS)
	$(LD) -lc $(LDFLAGS) $(OBJS) -o "$(PROJECT)"

.c.o:
	$(CC) --std=c90 -c -g -Wall -Werror -Wconversion -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 $(CFLAGS) src/$*.c

clean:
	rm -f *.o "$(PROJECT)"

install: build
	install "$(PROJECT)" "$(PREFIX)/bin"

test:
	sh tests/all.sh
