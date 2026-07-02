# Building a Dynamic Memory Allocator in C

## Overview

This project implements a simplified dynamic memory allocator in C.

The allocator supports:

- `my_malloc()`
- `my_free()`
- `my_realloc()`
- `my_calloc()`
- block splitting
- block coalescing
- 8-byte alignment
- heap visualization
- heap invariant verification

Rather than replicating the complexity of production allocators, the objective was to understand the fundamental problems every allocator must solve: managing heap memory, reusing freed blocks, reducing fragmentation, and maintaining correctness.

The allocator was developed incrementally. Each feature addressed a limitation discovered in the previous implementation, evolving from a simple heap-growing allocator into one capable of efficient memory reuse and structural verification.

---

## Design Goals

This allocator was designed as an educational systems programming project with an emphasis on understanding allocator design rather than maximizing performance.

The implementation prioritizes:

- correctness before optimization
- explicit heap metadata
- simple allocation strategies
- readable C code
- heap visualization for debugging
- invariant verification

The allocator is single-threaded and uses `sbrk()` to extend the heap. Although modern allocators employ considerably more sophisticated techniques, `sbrk()` exposes the underlying heap model and keeps the implementation focused on allocator fundamentals.

---

## 1. Growing the Heap

The first version of the allocator solved a single problem: returning memory to the caller.

Each allocation requested additional heap space from the operating system using `sbrk()`.

```c
void *raw = sbrk(sizeof(struct block) + size);
```

Each allocation consists of two parts:

```
+----------------+----------------------+
|    Metadata    |     User Memory      |
+----------------+----------------------+
^
|
block
```

The metadata stores information required to manage the allocation after it has been returned to the user.

```c
struct block {
    size_t size;
    int free;
    struct block *next;
    struct block *prev;
};
```

`my_malloc()` returns a pointer to the beginning of the user memory rather than the metadata.

```
+----------------+----------------------+
|    Metadata    |     User Memory      |
+----------------+----------------------+
                  ^
                  |
             returned pointer
```

Recovering the metadata later is simply a matter of subtracting one `struct block` from the user pointer.

This implementation was functional, but it had a fundamental limitation: every allocation permanently increased the size of the heap because freed memory could not be reused.

That limitation motivated the next iteration of the allocator.

---

## 2. Reusing Freed Memory

The first version of the allocator satisfied allocation requests, but every call to `my_malloc()` extended the heap. Once memory was freed, there was no way to reuse it.

The allocator needed a persistent view of the heap.

To achieve this, I introduced a doubly linked list that stores every block in heap order.

```text
head
 ↓
+------+    +------+    +------+
|Block |<-->|Block |<-->|Block |
+------+    +------+    +------+
                               ↑
                              tail
```

One of the earliest design decisions was how the allocator should traverse the list. I considered whether searches should begin from the oldest or newest blocks and whether introducing a doubly linked list required changing the logical direction of traversal.

Working through these questions led to an important distinction. The linked list represents the physical layout of the heap, while traversal strategy is simply an implementation choice.

The final design preserves heap order while searching from the tail toward the head, favoring recently freed blocks without changing the underlying memory layout.

With this structure in place, `my_malloc()` first searches for a reusable free block before requesting additional heap memory.

```text
Allocation Request
        │
        ▼
Find reusable free block?
        │
   ┌────┴────┐
   │         │
 Yes         No
   │         │
Reuse      sbrk()
```

For the first time, the allocator could recycle existing memory instead of growing the heap for every allocation.

---

## 3. Reducing Internal Fragmentation

Reusing freed blocks prevented unnecessary heap growth, but it also revealed a new problem.

Initially, any free block large enough to satisfy a request was reused in its entirety. While correct, this meant a small allocation could permanently consume a much larger free block.

```text
Requested: 24 bytes

Free block: 104 bytes

↓

Entire 104-byte block reused
```

Although no additional heap memory was requested, the remaining space became unavailable for future allocations. This introduced **internal fragmentation**.

To reduce this waste, I implemented block splitting.

Whenever the remaining space is large enough to hold both another metadata structure and another allocation, the allocator divides the block into two.

```text
Before

+---------+---------------------------+
| Meta    | 104 bytes (FREE)          |
+---------+---------------------------+

↓

After

+---------+------------+
| Meta    | 24 bytes   |
+---------+------------+
+---------+----------------------+
| Meta    | 48 bytes (FREE)      |
+---------+----------------------+
```

Implementing block splitting exposed another design decision. My initial instinct was to reason about the user pointer returned by `my_malloc()`. However, every allocator operation—splitting, merging, and resizing—operates on block metadata rather than user memory.

Once I began treating the metadata as the primary object, the implementation became much simpler. Calculating the location of the new block became a straightforward pointer arithmetic calculation.

```c
struct block *new_block =
    (struct block *)((char *)(block + 1) + size);
```

The allocator only performs a split when the remaining space is large enough to form a valid block. Otherwise, the entire free block is reused.

Block splitting significantly improved memory reuse, but it also introduced another problem. As blocks were repeatedly split and freed, the heap gradually became fragmented.

---

## 4. Reducing External Fragmentation

Block splitting made better use of large free blocks, but repeated allocations and deallocations gradually fragmented the heap.

A typical heap layout began to look like this:

```text
+---------+------------+
| Meta    | USED       |
+---------+------------+
+---------+------------+
| Meta    | FREE       |
+---------+------------+
+---------+------------+
| Meta    | FREE       |
+---------+------------+
+---------+------------+
| Meta    | USED       |
+---------+------------+
```

Although enough memory existed overall, it was divided into multiple smaller blocks. A future request for a larger allocation could fail despite sufficient contiguous memory being available.

This exposed the next limitation of the allocator: adjacent free blocks should not remain separate.

To address this, I implemented block coalescing.

Whenever neighboring blocks become free, they are merged into a single larger block.

```text
Before

+---------+------------+
| Meta    | FREE       |
+---------+------------+
+---------+------------+
| Meta    | FREE       |
+---------+------------+

↓

After

+---------+-------------------------+
| Meta    | Combined FREE Block     |
+---------+-------------------------+
```

Implementing coalescing was straightforward because the allocator already maintained a doubly linked list. Once a block was freed, the allocator only needed to examine its neighboring blocks before updating the list.

One of the more useful debugging tools during this stage was heap visualization. Rather than inspecting pointers manually, I could observe how the heap changed after every allocation, split, and free operation. This made fragmentation much easier to reason about and helped expose cases where adjacent free blocks should have been merged.

Reducing fragmentation became about more than reusing memory; it also meant preserving contiguous regions large enough to satisfy future allocation requests.

---

## 5. Ensuring Memory Alignment

By this stage, the allocator correctly reused, split, and merged heap blocks. The remaining issue was ensuring that every allocation satisfied the platform's alignment requirements.

To guarantee proper alignment, every allocation request is rounded up to the nearest multiple of eight.

```c
size = align8(size);
```

where

```c
size_t align8(size_t size) {
    return (size + 7) & ~7;
}
```

Although alignment slightly increases the size of some allocations, it guarantees that every pointer returned by the allocator satisfies the expected alignment requirements of the platform.

---

## 6. Supporting Dynamic Reallocation

Unlike the previous allocator functions, implementing `realloc()` was not a matter of applying a single algorithm. The allocator had to choose between multiple strategies depending on the current heap layout while preserving the existing data.

### Shrinking an Allocation

Shrinking an allocation is the simplest case. Since the requested size is smaller than the current block, the existing memory can be reused without moving any data.

If enough space remains after shrinking, the allocator splits the block and returns the remaining space to the free list.

```text
Before

+---------+-----------------------------+
| Meta    | 64 bytes (USED)             |
+---------+-----------------------------+

↓

After

+---------+------------+
| Meta    | 16 bytes   |
+---------+------------+
+---------+----------------+
| Meta    | FREE           |
+---------+----------------+
```

If the remaining space is too small to form a valid block, the allocator leaves the block unchanged. This avoids creating unusable fragments while keeping the implementation simple.

### Growing an Allocation

For growing an allocation, I considered always allocating a new block, copying the existing data, and freeing the old block. While correct, this would move memory even when sufficient space already existed. Instead, the allocator first attempts to grow the allocation in place by reusing the following free block.

If the combined space is large enough, the two blocks are merged and the allocation grows in place without copying any data:

```
+---------+------------+
| Meta    | 16 bytes   |
+---------+------------+
+---------+----------------+
| Meta    | FREE           |
+---------+----------------+

↓

After

+---------+-----------------------------+
| Meta    | 64 bytes (USED)             |
+---------+-----------------------------+
```

If in-place expansion is not possible, the allocator falls back to allocating a new block, copying the existing contents, and freeing the original allocation.

```text
Allocate new block

↓

Copy existing data

↓

Free old block
```

Testing also exposed another interaction between allocator components. Shrinking a block through `realloc()` creates a new free block, which may become immediately adjacent to an existing free block. Rather than leaving two consecutive free blocks, the allocator coalesces them to preserve the invariant that adjacent free blocks should never exist.

Implementing `realloc()` reinforced that allocator features are rarely independent; supporting one operation often requires revisiting and strengthening existing ones to maintain a consistent heap.

---

## 7. Verifying Correctness

As the allocator grew more complex, debugging became increasingly difficult. A single incorrect pointer update during splitting or coalescing could silently corrupt the heap while still appearing to work for simple test cases.

Heap visualization made it easier to understand allocator behavior, but it did not guarantee correctness. Instead of asking whether the heap "looked" correct, I began asking what properties should always hold.

This led to the introduction of heap invariant verification.

After every operation during testing, the allocator validates properties such as:

- the head block has no previous block
- the tail block has no next block
- every block has a positive size
- neighboring pointers are consistent
- adjacent free blocks never exist
- the linked list matches the physical heap layout

For example, the allocator verifies that each block immediately follows the previous block in memory.

```c
struct block *expected =
    (struct block *)((char *)(current + 1) + current->size);

assert(current->next == expected);
```

These invariants transformed debugging from manual inspection into automatic verification. Rather than allowing structural bugs to propagate into undefined behavior, the allocator detects inconsistencies immediately during testing.

More importantly, this shifted my focus from validating individual allocator operations to validating the correctness of the heap as a whole.

---

## Allocator Workflow

```
malloc()
    │
Find free block?
 ├── Yes → Split if needed → Return
 └── No  → sbrk() → Return

free()
    │
Mark free
    │
Coalesce

realloc()
    │
Shrink? → Split
Grow?   → Expand in place
Else    → Allocate → Copy → Free
```

## Lessons Learned

Building a memory allocator reinforced that systems programming is an iterative design process. Every improvement solved one limitation while introducing another, requiring the allocator to evolve incrementally rather than through a single implementation.

The project also changed how I approached debugging. Heap visualization made allocator behavior easier to understand, but invariant verification provided confidence that the allocator remained structurally correct after every operation.

More broadly, this project strengthened my understanding of heap organization, metadata management, fragmentation, pointer arithmetic, and maintaining correctness in pointer-heavy systems code. It also demonstrated how seemingly independent allocator features evolve into a tightly coupled system whose correctness depends on maintaining clear invariants.