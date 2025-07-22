CC      = $(CC)
CFLAGS  = -std=c11 -O3
LDFLAGS =

.PHONY: all clean

all: clean lib

%.o: %.c
	$(CC) -o $@ $^ -c $(CFLAGS) $(LDFLAGS)

lib: noise1234.o
	ar rcs libnoise.a noise1234.o

clean:
	rm -rf libnoise.a