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

The queue is shared by worker threads consuming jobs and threads submitting new jobs.

A mutex protects:

- queue modifications
- shutdown state

Only one thread may access this shared state at a time.

Workers sleep while the queue is empty and recheck the queue after waking before dequeuing work.

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

## Reflection

This project clarified that concurrency is primarily a shared-state coordination problem. The mutex protects the queue and shutdown state, while the condition variable allows workers to wait efficiently until that state may have changed.

The implementation also reinforced the importance of ownership and lifetime. Submitted jobs are created by the pool, temporarily owned by the queue, executed and freed by workers, while the worker threads themselves remain alive until graceful shutdown. Encapsulating these responsibilities behind a small API made the thread pool reusable without exposing its synchronization details to callers.