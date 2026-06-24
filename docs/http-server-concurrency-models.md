# Building HTTP Servers in C: Processes, Threads, and Event Loops

## Overview

This project implements the same TCP-based HTTP file server using three different concurrency architectures:

- Process-per-connection server (`fork`)
- Thread-per-connection server (`pthread`)
- Event-driven server (`select`)

All three implementations support:

- HTTP request parsing
- Static file serving
- Content-Type detection
- Basic request validation
- 403 and 405 responses
- HTML, CSS, JavaScript, and PNG files

Rather than focusing on web development features, the primary objective was to understand how operating system abstractions influence scalability, isolation, coordination, and resource utilization in networked systems.

By keeping the HTTP functionality constant while changing only the concurrency architecture, the project provides a direct comparison of three fundamental approaches used in server design.

### Project Progression

```Plain text
HTTP File Server
↓
Process-per-Connection  (http_server.c)
↓
Thread-per-Connection   (http_server_thread.c)
↓
Event-Driven Server     (http_server_select.c)
```

## Shared HTTP Workflow

All three implementations utilize the request-processing pipeline below:

```Plain text
Client Request  
↓
Parse HTTP Request
↓
Validate Request  
↓
Resolve File Path  
↓
Determine Content Type  
↓
Send HTTP Response  
↓
Close Connection
```

Core HTTP functionality was intentionally reused across all three implementations through:
- `parse_request()`
- `build_file_path()`
- `get_content_type()`
- `send_file_response()` 

This separation made it possible to compare concurrency architectures independently while keeping application behavior constant.

---

## 1. Process-per-Connection Server

### Architecture

```Plain text
accept()
↓
fork()
├── Parent Process → continue accepting clients
└── Child Process  → handle HTTP request
```

Each client connection is handled by a separate child process created by `fork()`.

### Key Observations

This architecture uses process isolation as its primary concurrency mechanism. Each client executes within its own virtual address space, eliminating shared-state concerns and greatly simplifying correctness.

Implementing this version required managing child lifecycles, preventing zombie processes, and coordinating cleanup through `SIGCHLD` and `waitpid()`.

While straightforward to reason about, the model incurs significant overhead because each connection requires process creation, scheduling, and memory isolation.

### Tradeoffs

Advantages:
- Strong client isolation
- Avoids shared-state bugs
- Simple concurrency model

Limitations:
- High memory overhead
- Expensive process creation
- Poor scalability under large connection counts

---

## 2. Thread-per-Connection Server

### Architecture

```Plain text
accept()
↓
pthread_create()
↓
Worker Thread
```

Each client connection is handled by a dedicated thread within the same process.

### Key Observations

The thread-per-connection architecture reduced the overhead associated with process creation by allowing clients to share a single address space.

However, it also introduced new correctness challenges such as race conditions during development. Since correctness was no longer guaranteed through isolation, explicit synchronization through mutexes was required.

Overall, this version demonstrated that reducing resource costs often comes at the expense of increased coordination complexity.

### Tradeoffs

Pros:
- Lower memory and creation overhead than processes
- Faster creation and context switching
- Easy communication through shared memory

Cons:
- Synchronization complexity
- Race condition handling
- Undefined behavior in one thread can impact the entire process

---

## 3. Event-Driven Server

### Architecture

```Plain text
listen_fd
client_fd_1         select()        Process Ready
client_fd_2         ------->        File Descriptors
client_fd_3
    ...
```

A single event loop manages multiple client connections without creating additional processes or threads.

### Key Observations

This implementation uses I/O multiplexing rather than execution parallelism. Instead of assigning resources per connection, a single thread reacts to readiness notifications from the kernel.

Building this version required managing file descriptor sets, tracking active connections, maintaining readiness state, and understanding how sockets transition between connected, readable, and disconnected states.

One particularly important observation was that a disconnected socket is also considered readable. A subsequent read() returns EOF immediately, making disconnect handling part of normal event processing rather than a separate event type.

This architecture dramatically reduces per-connection overhead while shifting complexity into explicit state and connection management.

### Tradeoffs

Pros:
- No per-client processes or threads
- Lower memory usage
- Lower per-connection overhead

Cons:
- More complex control flow
- Manual connection management
- Event loop complexity grows with application features

---

## Comparing the Approaches

| Model | Concurrency Mechanism | Isolation | Resource Cost | Complexity |
|---------|---------|---------|---------|---------|
| Process-per-Connection | `fork()` | High | High | Low |
| Thread-per-Connection | `pthread_create()` | Moderate | Moderate | Moderate |
| Event-Driven | `select()` | Low | Low | High |

---

## Security Considerations

Although this project focused primarily on systems programming, several security-relevant concerns emerged during implementation:

- Path traversal prevention through rejection of `..` sequences
- HTTP method validation and restriction
- Explicit buffer size management during request parsing
- Safe construction of filesystem paths from user-controlled input
- Correct handling of file descriptor lifecycles
- Detection of disconnect and EOF conditions
- Separation between network-facing input and filesystem access

These concerns reinforced the close relationship between systems programming and secure software engineering.

---

## Systems Concepts Explored

- Process creation, isolation, and lifecycle management
- Thread creation and shared-memory concurrency
- Race conditions and synchronization with mutexes
- File descriptor management
- TCP socket programming
- HTTP request parsing and response generation
- Event-driven I/O and readiness-based processing
- Signal handling and child process cleanup
- Basic web security controls and input validation

---

## Technical Takeaways

Implementing the same HTTP server across three concurrency architectures highlighted that improvements in scalability, isolation, or resource efficiency are often competing goals:

- Processes simplify correctness through isolation but incur significant overhead.
- Threads reduce overhead but introduce synchronization challenges.
- Event-driven architectures minimize per-connection resource costs but require explicit state management and more complex control flow.

While exploring these tradeoffs through working code was valuable, the project also reinforced an understanding of why different designs exist in the first place. Processes, threads, and event loops all solve the same problem, but they do so with different assumptions, strengths, and limitations. Implementing each approach helped develop intuition for reasoning about concurrency, resource management, and system design—concepts that underpin systems software, backend infrastructure, distributed systems, and security-sensitive applications.