# HashMap in C

## Goal

This project is a hash map implementation in C built as part of my systems programming learning journey. The goal was to understand how hash tables work internally by implementing the data structure from scratch, with an emphasis on pointers, dynamic memory management, collision handling, and resizing.

The implementation supports insertion, lookup, update, removal, and automatic resizing while storing arbitrary values through `void *`.

---

## Features

- Separate chaining using linked lists
- Automatic resizing
- String keys with copied ownership
- Generic `void *` values
- Insert, lookup, update, and removal
- Unit-tested
- Verified with AddressSanitizer

---

## Project Goals

The primary objective of this project was to strengthen my understanding of systems programming concepts in C rather than build a production-ready library.

Topics explored include:

- Pointer manipulation
- Dynamic memory management
- Linked lists
- Hash table implementation
- Memory ownership
- API design
- Unit testing

---

## Public API

```c
struct hashmap *hashmap_create(size_t bucket_count);

void hashmap_destroy(struct hashmap *map);

bool hashmap_put(struct hashmap *map,
                 const char *key,
                 void *value);

void *hashmap_get(struct hashmap *map,
                  const char *key);

bool hashmap_remove(struct hashmap *map,
                    const char *key);
```

| Function | Description |
|----------|-------------|
| `hashmap_create()` | Creates a new hash map. |
| `hashmap_destroy()` | Frees all allocated memory. |
| `hashmap_put()` | Inserts a new key or updates an existing key. |
| `hashmap_get()` | Returns the stored value or `NULL` if the key does not exist. |
| `hashmap_remove()` | Removes a key from the map. |

---

## Implementation

The hash map stores entries in an array of buckets.

Each bucket contains a singly linked list used for separate chaining when collisions occur.

Keys are hashed using the **djb2** hash function. When the number of stored entries reaches the number of buckets, the bucket array is resized and all existing entries are rehashed into the new table.

Keys are copied into the map, while values are stored as `void *` without taking ownership of the underlying data.

---

## Design Decisions

### Separate Chaining

Collisions are resolved using linked lists instead of open addressing. This keeps insertion and removal straightforward while providing good average-case performance.

### Key Ownership

Keys are duplicated during insertion so callers may safely modify or free the original string after calling `hashmap_put()`.

### Value Ownership

Values are stored as `void *` and are **not** copied or freed by the hash map. Memory ownership remains with the caller, allowing the map to store arbitrary user-defined data.

### Resize Strategy

The table resizes when the number of entries equals the number of buckets (`entry_count == bucket_count`). During resizing, existing entries are rehashed into a larger bucket array.

---

## Complexity

| Operation | Average |
|-----------|--------:|
| Insert | O(1) |
| Lookup | O(1) |
| Update | O(1) |
| Remove | O(1) |
| Resize | O(n) |

---

## Testing

The implementation was tested for:

- Hash map creation and destruction
- Insert and lookup
- Updating existing keys
- Key removal
- Missing keys
- Collision handling
- Collision removal
- Automatic resizing
- Large-scale insertion and removal stress tests

The test suite was also executed with **AddressSanitizer** to check for common memory errors such as invalid accesses, use-after-free, double-free, and memory leaks.

---

## Build

Compile the implementation and test suite:

```bash
gcc -Wall -Wextra -Wpedantic -std=c11 \
    hashmap.c tests/hashmap_tests.c \
    -o hashmap_tests
```

Run the tests:

```bash
./hashmap_tests
```

---

## Reflection

This project helped me better understand how commonly used data structures are implemented beneath the standard library. While I was already familiar with hash maps conceptually, implementing one from scratch made pointer manipulation, memory ownership, collision handling, and resizing much more intuitive.

One of the biggest takeaways was the importance of defining clear ownership rules. Deciding when to copy data, when to transfer ownership, and when to leave ownership with the caller became just as important as implementing the data structure itself.

Overall, this project strengthened my understanding of dynamic memory management and reinforced the value of thorough testing and debugging when writing systems-level software.