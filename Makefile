CC = gcc
CFLAGS = -O2 -Wall -Wextra -fPIC -std=c99

SRC = csrc/bloom.c
OBJ = csrc/bloom.o

.PHONY: all clean test-c

all: $(OBJ)

$(OBJ): $(SRC) csrc/bloom.h
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ) -lm

clean:
	rm -f csrc/*.o

test-c:
	$(CC) $(CFLAGS) -o csrc/test_c csrc/bloom.c csrc/test_main.c -lm
	./csrc/test_c
