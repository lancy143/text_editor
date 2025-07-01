CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99
SRCS = server.c client.c wrap.c
OBJS = $(SRCS:.c=.o)
all: main
main: main.o
	$(CC) $^ -o $@ $(CFLAGS)
editor: editor.o
	$(CC) $^ -o $@ $(CFLAGS)
editor_core: editor_core.o
	$(CC) $^ -o $@ $(CFLAGS)
input: input.o
	$(CC) $^ -o $@ $(CFLAGS)
file_io: file_io.o
	$(CC) $^ -o $@ $(CFLAGS)
buffer: buffer.o
	$(CC) $^ -o $@ $(CFLAGS)
display: display.o
	$(CC) $^ -o $@ $(CFLAGS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
clean:
	rm -f $(OBJS) main editor editor_core input file_io buffer display