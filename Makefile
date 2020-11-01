CC=clang
CFLAGS=-Wall -Wextra -pedantic
LDFLAGS=-D_REENTRANT -I/usr/local/include/taglib -I/usr/include/SDL2 -L/usr/local/lib -ltag_c -ltag -lz -lSDL2_mixer -lSDL2 -lm -lncursesw -lstdc++
BIN=tap
SRCDIR=./src
SRC=$(SRCDIR)/main.c

all: $(SRC)
	$(CC) $^ -o $(BIN) $(CFLAGS) $(LDFLAGS)
