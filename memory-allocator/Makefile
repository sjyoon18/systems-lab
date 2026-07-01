CC = gcc
CFLAGS = -Wall -Wextra -g

BIN = bin

all: $(BIN) \
	$(BIN)/malloc_test \
	$(BIN)/split_test \
	$(BIN)/coalesce_test \
	$(BIN)/realloc_test \
	$(BIN)/calloc_test

$(BIN):
	mkdir -p $(BIN)

$(BIN)/malloc_test: allocator.c tests/malloc_test.c
	$(CC) $(CFLAGS) allocator.c tests/malloc_test.c -o $@

$(BIN)/split_test: allocator.c tests/split_test.c
	$(CC) $(CFLAGS) allocator.c tests/split_test.c -o $@

$(BIN)/coalesce_test: allocator.c tests/coalesce_test.c
	$(CC) $(CFLAGS) allocator.c tests/coalesce_test.c -o $@

$(BIN)/realloc_test: allocator.c tests/realloc_test.c
	$(CC) $(CFLAGS) allocator.c tests/realloc_test.c -o $@

$(BIN)/calloc_test: allocator.c tests/calloc_test.c
	$(CC) $(CFLAGS) allocator.c tests/calloc_test.c -o $@

clean:
	rm -rf $(BIN)