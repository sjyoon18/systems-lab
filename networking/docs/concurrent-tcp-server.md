# Concurrent TCP Echo Server

## Goal
Implement a TCP echo server that can handle multiple clients concurrently.

The server accepts client connections on port 8080 and for each new client connected, it creates a separate child process using `fork()`. Each child handles one client connection by reading from the socket and writing the same data back.

Concepts explored:
- TCP sockets
- file descriptors
- `accept()`
- `fork()`
- `SIGCHLD`
- `waitpid()`
- zombie process cleanup

---

## Motivation

A single-process echo server can only actively handle one client connection at a time:

```text
Client A connects
        ↓
Server handles A
        ↓
Client B waits until Client A disconnects
```

To handle multiple clients, the server must separate connection acceptance from client handling.

This implementation creates an individual child process for each client connection accepted at the parent process:

```text
Client A → Child A
Client B → Child B
Client C → Child C
...
```

## Architecture

```text
                    Parent Process
                     (listen_fd)
                          |
                          |
                        accept()
                          |
                        fork()
                     /          \
                    /            \
                   v              v

           Child Process    Parent Process
            (client_fd)       (listen_fd)
                  |                 |
                  |                 |
            handle client           |
            read/write              |
            close/exit              |
                                    v
                            accept next client
                                    .
                                    .
```
- The parent owns the listening socket and continues to accept new clients
- Each child owns one client-connected socket and handles the client.

---

## Concurrent Server Lifecycle

```text
socket()
 ↓
bind()
 ↓
listen()
 ↓
accept()
 ↓
fork()
 ↓
Parent continues accepting clients
Child handles connected client

```

- `socket()` creates a kernel-managed socket object and returns a file descriptor referring to that object.
- `bind()` associates the socket with an IP address and port.
- `listen()` marks the socket as a listening socket.
- `accept()` blocks until a client connects, then returns a file descriptor for a new connected socket.
- `fork()` creates a child process to handle that client, allowing multiple client handling:
```text
fork()
    |
    +--> parent  -> continues accepting clients
    |
    +--> child   -> handles the accepted client
```

---

## File Descriptor Ownership After `fork()`

After `fork()`, the parent and child both inherit the same file descriptor references:

For example:
```text
Parent:
    listen_fd = 3
    client_fd = 4
Child:
    listen_fd = 3
    client_fd = 4
```

Each process closes unused file descriptors according to their purpose:

```text
Parent:
    keeps listen_fd
    closes client_fd
Child:
    closes listen_fd
    keeps client_fd
```

This prevents unintended resource leaks and ensures correct connection cleanup.

---

## Echo Communication Flow

Each child process executes an echo loop:

```text
Client stdin
    ↓
write(sockfd)
    ↓
TCP connection
    ↓
Server child read(client_fd)
    ↓
Server child write(client_fd)
    ↓
TCP connection
    ↓
Client read(sockfd)
    ↓
Client stdout
```

The server simply reads bytes from the connected client socket and writes the same bytes back.

---

## Child Exit and Zombie Processes

When the client disconnects:

```text
Client closes socket
        ↓
Server read() returns 0
        ↓
Child exits
```

The child process no longer executes but the kernel temporarily retains:

```text
PID
Exit status
Termination information
```
of the exited child process until the parent collects the information with `wait()` or `waitpid()`.

Zombie processes consume entries in the kernel process table. If they accumulate indefinitely, system resources can eventually be exhausted.

---

## `SIGCHLD` and Reaping Children

The server installs a signal handler:

```c
signal(SIGCHLD, reap_children);
```

and the handler calls:
```c
while(waitpid(-1, NULL, WNOHANG) > 0) {
}
```

so when child exits:

```text
Child exits
    ↓
Kernel sends SIGCHLD
    ↓
Parent receives signal
    ↓
SIGCHLD handler executes
    ↓
waitpid() reaps exited child
```

The signal handler temporarily interrupts the parent process, performs cleanup, and then execution resumes from the interrupted point.

---

### Why `while(waitpid(..., WNOHANG))`?

multiple child exits may be represented by fewer `SIGCHLD` deliveries because signals can be coalesced.

For example:
```text
Child A exits
Child B exits
Child C exits
      ↓
SIGCHLD delivered
```

Thus the handler repeatedly calls:

```c
while (waitpid(-1, NULL, WNOHANG) > 0) {
}
```
to reap all exited children.

`WNOHANG` prevents the handler from blocking when no exited children remain. This allows `waitpid()` to return immediately and the parent resumes its normal execution.

---

## Execution Model 

```text
Parent blocks in accept()
        ↓
Client connects
        ↓
accept() returns
        ↓
fork()
        ↓
Parent closes client_fd and continues accepting
        ↓
Child closes listen_fd and handles client
        ↓
Child exits
        ↓
Kernel sends SIGCHLD
        ↓
Parent reaps child with waitpid()
```

---

## Unix Abstraction Insight

A major observation from this project was that Unix exposes many fundamentally different resources through the same abstraction:

```text
File        -> read() / write()
Pipe        -> read() / write()
TCP Socket  -> read() / write()
```

Although files, pipes, and sockets represent different kernel-managed objects, each is accessed through a file descriptor with the same `read()`/`write()` interface.

---

## Reflection

This project connected process management with network programming. Each child process owns one connected socket, while the parent remains responsible for accepting new clients and reaping terminated children.

The implementation also reinforced the Unix file-descriptor model. Files, pipes, and sockets represent different kernel objects, but the shared `read()` and `write()` interface makes them composable through the same process and ownership rules. Handling `SIGCHLD` and zombie cleanup showed that concurrency requires managing process lifetime as carefully as connection lifetime.