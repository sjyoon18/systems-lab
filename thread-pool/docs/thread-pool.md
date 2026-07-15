# Thread Pool

## Goal

Implement a reusable thread pool that executes submitted jobs using a fixed number of worker threads.

Instead of creating a new thread for every task, worker threads are created once during initialization and continuously process jobs from a shared queue.

---

## Motivation

Creating a thread for every request introduces unnecessary overhead and makes resource usage unpredictable under heavy load.

A thread pool provides:

- bounded thread count
- lower thread creation overhead
- predictable CPU and memory usage
- reusable workers for concurrent workloads

---

## Architecture

```
                    thread_pool_init()

                           │

          ┌────────────────┼────────────────┐
          ▼                ▼                ▼

      Worker 1        Worker 2        Worker N
              (sleeping on condition variable)

                           ▲
                           │

                  thread_pool_submit()

                           │

                           ▼

                     Shared Job Queue
```

Each submitted job consists of:

- function pointer
- function argument

Workers repeatedly:

1. wait for work
2. dequeue a job
3. execute the function
4. free the completed job
5. wait for more work

---

## Job Queue

Jobs are stored in a FIFO linked list.

```c
struct job {
    void (*function)(void *);
    void *arg;
    struct job *next;
};
```

The queue maintains both a head and tail pointer, allowing constant-time enqueue and dequeue operations.

---

## Synchronization

The queue is shared by all worker threads and producers.

A mutex protects:

- queue modifications
- shutdown state

Only one thread may access this shared state at a time.

Workers avoid busy waiting by sleeping on a condition variable whenever the queue is empty.

```text
queue empty
        │
        ▼
pthread_cond_wait()

        │
        ▼
worker sleeps

        │
 thread_pool_submit()

        │
        ▼
pthread_cond_signal()

        │
        ▼
worker wakes
```

The condition variable allows workers to remain idle without continuously polling the queue.

---

## Graceful Shutdown

Destroying the pool does not terminate workers immediately.

Instead:

1. stop accepting new jobs
2. wake sleeping workers
3. finish all queued work
4. allow workers to exit
5. join every worker thread
6. destroy synchronization primitives

This guarantees that submitted jobs are completed before the pool is destroyed.

---

## Concepts Practiced

- producer-consumer architecture
- linked-list queues
- mutexes
- condition variables
- thread lifecycle
- ownership and resource cleanup
- graceful shutdown
- reusable concurrent API