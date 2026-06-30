CC = gcc
CFLAGS = -O2 -Wall -Wextra -fPIC -std=c99

SRC = csrc/bloom.c
OBJ = csrc/bloom.o

.PHONY: all clean dev lint test

all: $(OBJ)

$(OBJ): $(SRC) csrc/bloom.h
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ) -lm

dev:
	uv pip install -e ".[dev]" --no-build-isolation
	uv run python setup.py build_ext --inplace

lint:
	uv run ruff check .
	uv run ruff format --check .

clean:
	rm -f csrc/*.o
	rm -f _cbloom*.so
	rm -rf build/
	rm -rf *.egg-info

test:
	$(CC) $(CFLAGS) -o csrc/test_c csrc/bloom.c csrc/test_main.c -lm
	./csrc/test_c
