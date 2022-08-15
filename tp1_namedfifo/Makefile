# CFLAGS = -Wall

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

.PHONY: all clean

all: $(OBJS)

clean:
	rm -f *.o tp1_fifo Sign.txt Log.txt

.c.o:
	gcc $< -o $@ 

