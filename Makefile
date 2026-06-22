CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Isrc $(shell pkg-config --cflags ncurses)
LDFLAGS = $(shell pkg-config --libs ncurses)

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

file-magik: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f src/*.o file-magik
