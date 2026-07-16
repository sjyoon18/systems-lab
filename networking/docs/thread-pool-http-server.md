# Thread Pool HTTP Server

## Goal

Implement an HTTP server that processes client connections using a reusable thread pool instead of creating a new thread for every request.

Incoming connections are submitted as jobs to the thread pool, allowing a fixed number of worker threads to process requests concurrently.

---

## Motivation

A thread-per-request server creates a new thread for every incoming client.

Although simple, this approach increases thread creation overhead and allows resource usage to grow with the number of connections.

Replacing per-request thread creation with a thread pool provides:

- bounded thread count
- predictable memory usage
- lower scheduling overhead
- reusable worker threads
- cleaner separation between networking and concurrency

---

## Architecture

```
Browser

    │
    ▼

accept()

    │
    ▼

thread_pool_submit()

    │
    ▼

Shared Job Queue

    │
    ▼

Worker Thread

    │
    ▼

handle_client()

    │
    ▼

HTTP Response
```

The main thread only accepts incoming connections and submits work.

Worker threads perform all request processing.

---

## Connection Lifecycle

Each accepted socket descriptor is copied into dynamically allocated memory before submission.

```c
int *client_fd_ptr = malloc(sizeof(int));
```

The submitted job owns this allocation.

The worker:

1. copies the descriptor
2. frees the allocated argument
3. handles the HTTP request
4. closes the client socket

This avoids multiple jobs referencing the same stack variable.

---

## Graceful Shutdown

The server installs a `SIGINT` handler that requests shutdown without performing cleanup directly.

When `Ctrl+C` is received:

1. the signal handler sets a shutdown flag
2. `accept()` is interrupted
3. the accept loop exits
4. the listening socket is closed
5. `thread_pool_destroy()` finishes queued work
6. worker threads exit and are joined

This allows active requests to complete before the server terminates.

---

## Modular Design

The server separates responsibilities into independent modules.

```
thread_pool.c
    Worker management
    Job scheduling
    Synchronization

http_common.c
    HTTP parsing
    File serving
    Response generation

thread_pool_http_server.c
    Socket management
    Accept loop
    Signal handling
    Thread pool integration
```

This separation allows the same HTTP implementation to be reused across different server architectures.

---

## Reflection

Integrating the thread pool changed the HTTP server from managing threads directly to submitting units of work through a reusable concurrency layer. The accept loop now focuses on connections, while the pool owns worker creation, synchronization, scheduling, and shutdown.

The project also made ownership boundaries more concrete. Each accepted descriptor is passed through a separately allocated argument, transferred to a worker, and closed after request handling. Separating HTTP processing, socket lifecycle, and thread management made the server easier to reason about and allowed the same HTTP module to support multiple concurrency architectures.