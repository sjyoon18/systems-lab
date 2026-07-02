# Unix Pipeline with C

## Goal

Implement support for piped commands such as:

```bash
ls | grep txt
```

using:
- `fork()`
- `pipe()`
- `dup2()`
- `execvp()`
- `wait()`

---

## Background

A Unix pipeline connects the standard output of one process to the standard input of another process.

```Plain text
stdout                stdin
ls  -------------->  grep txt
```

- `ls` writes into a pipe instead of writing to terminal (fd = 1).
- `grep` reads from the pipe instead of reading from terminal (fd = 0).

---

## Process Structure

The shell:

1. Creates a pipe (`pipe()`)
2. Creates a child process for the left command (`fork()`)
3. Creates a child process for the right command (`fork()`)

```Plain text
Shell
  ├── Child 1 (ls)
  └── Child 2 (grep txt)
```

4. Waits for both children to terminate.

---

## Necessity of `fork()`

For example:
```Bash
ls | grep txt
```
- `ls` produces data.
- `grep` consumes data.

As demonstrated in the example, piped commands require multiple programs to execute concurrently; the shell's use of `fork()` allows both commands to execute simultaneously in separate child processes while communicating through the pipe.

---

## Creating the Pipe

```c
int fd[2];
pipe(fd);
```

After the call:
```Plain text
fd[0] = read end
fd[1] = write end 
```
The pipe behaves as a kernel-managed byte stream.

---

## Redirecting `stdout` and `stdin` with `dup2()`

Programs such as `ls` or `grep` simply:
- read from stdin (fd = 0)
- write to stdout (fd = 1)

They do not recognize that they are part of a pipeline.

`dup2()` changes what those file descriptors point to.

---

For the left command, after:

```c
dup2(fd[1], 1);
```

Any output written to standard output (fd = 1) enters the pipe write end (fd[1]).

For the right command, after:

```c
dup2(fd[0], 0); 
```

Any input read from standard input (fd = 0) comes from the pipe read end (fd[0]).

---

## Executing Commands

After redirection:

```c
execvp(argv1[0], argv1); 
```

and

```c
execvp(argv2[0], argv2);
```

replace the child processes with the requested programs.

For example:

```Plain text
argv1 -> ls
argv2 -> grep txt
```

---

## File Descriptor Ownership

Child processes inherit the file descriptors of their parent process:

```Plain text
Child 1:
    fd[0]
    fd[1]
Child 2:
    fd[0]
    fd[1]
```

Unused pipe ends are closed after `dup2()` to prevent resource leaks and ensure correct EOF behavior:

```c
close(fd[0]);
close(fd[1]); 
```

If a write end remains open, the kernel assumes that additional data might arrive later.

---

## Example Execution

```bash
ls | grep txt
```

Execution flow:

```Plain text
Shell
 │ 
 ├── create pipe 
 │ 
 ├── fork child 
 │     └── stdout -> pipe 
 │     └── exec(ls) 
 │ 
 ├── fork child 
 │     └── stdin <- pipe 
 │     └── exec(grep txt) 
 │ 
 └── wait for both children 
 ```

---

## What I Learned

- A pipe is a kernel-managed byte stream.
- Bytes written to the write end are available at the read end following the FIFO principle.
- `fork()` creates the child processes.
- `dup2()` redirects standard input and output.
- `execvp()` replaces the process with a requested command.
- Child processes inherit file descriptors from their parent.
- Child processes that inherit the same pipe file descriptors can communicate through the pipe.
- A pipe reaches EOF when it is empty and all write ends have been properly closed.
- Unix pipelines are built from managing file descriptors and processes

---

## Repository Reference

Implementation:
```Plain text
shell/main.c
```

Commit:
```Plain text
3e334ea0bd37a2c444d6f27f2cf94642e28f0433
```