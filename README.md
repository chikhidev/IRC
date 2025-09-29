
# Linux TCP Sockets, `bind`/`listen`/`accept` Internals — _Dead-level_ (plus `epoll`)

> This document collects low-level kernel-facing details for the TCP server lifecycle:
> `socket()` → `bind()` → `listen()` → `accept()` and how `epoll` integrates for scalable I/O.
> It shows representative kernel structs, queue layouts and syscall-level behavior.
>
> **Note:** struct layouts are simplified and annotated for clarity. Real Linux kernel sources change across versions.
> Use this as a precise, concrete guide to the data-structures and steps inside the kernel.

---

## Table of contents
1. `socket()` — what happens in the kernel
2. `struct socket` vs `struct sock` vs `inet_sock` vs `tcp_sock`
3. `bind()` — what it sets (exact fields)
4. `listen()` — backlog, SYN queue and accept queue (`request_sock_queue`)
5. How a client connection flows (SYN → SYN+ACK → ACK) and how `request_sock` becomes `struct sock`
6. `accept()` — what kernel returns and how a new fd is created
7. Non-blocking sockets and `O_NONBLOCK`
8. `epoll` (epoll_create1/epoll_ctl/epoll_wait) — integration with sockets and edge vs level triggers
9. Minimal example: a non-blocking epoll-based TCP server (C)
10. `strace`-style syscall trace example
11. References & next steps for reading kernel code

---

## 1) `socket()` — syscall-level sequence

User code:
```c
int s = socket(AF_INET, SOCK_STREAM, 0);
```

Syscall flow in kernel (conceptual):
- `sys_socket()` -> `__socket()` -> `__sock_create()`  
- Kernel allocates:
  - a `struct socket` (generic wrapper)
  - a `struct sock` (protocol implementation for TCP)
  - a `struct file` associated with the process fd table referencing the socket

A very small conceptual fragment:

```c
/* simplified conceptual struct */
struct socket {
    unsigned int    state;      /* generic socket state */
    short           type;       /* SOCK_STREAM etc. */
    unsigned long   flags;
    struct file     *file;      /* userspace fd points to this file */
    struct sock     *sk;        /* protocol-specific object (TCP) */
};
```

The `struct file` entry is what the userspace fd (an integer) indexes into:
```
userspace fd -> struct file -> struct socket -> struct sock -> tcp/inet specifics
```

---

## 2) `struct sock`, `inet_sock`, `tcp_sock`

These are **kernel internal** protocol structures. Below are simplified but concrete representations:

```c
/* generic protocol-independent socket object */
struct sock {
    struct socket   *sk_socket;         /* back pointer to struct socket */
    const struct proto *sk_prot;        /* protocol ops (tcp, udp) */
    int              sk_state;          /* TCP state: TCP_LISTEN, TCP_ESTABLISHED... */
    spinlock_t       sk_lock;           /* protect socket data */
    struct inet_sock *sk_inet;          /* IPv4-specific wrapper */
    struct request_sock_queue sk_accept_queue; /* accept queue for listening sockets */
    /* many other fields: timers, rb-trees, skb queues, congestion control, etc */
};
```

IPv4-specific container:
```c
struct inet_sock {
    __be32 inet_saddr;    /* local IP (network byte order) */
    __be16 inet_sport;    /* local port (network byte order) */
    __be32 inet_daddr;    /* remote IP (for connected sockets) */
    __be16 inet_dport;    /* remote port */
    /* TTL, tos, rcv/snd buffer defaults, reuse flags, etc. */
};
```

TCP-specific socket (more fields in real kernel `struct tcp_sock`):
```c
struct tcp_sock {
    struct sock  sk;           /* inherits struct sock */
    /* TCP state machine fields: seq numbers, windows, timers */
    __u32        snd_una, snd_nxt, rcv_nxt;
    unsigned int snd_cwnd;     /* congestion window */
    /* ... */
};
```

**Important relationship**:
- `struct socket` is the generic user-facing wrapper.
- `struct sock` is the protocol implementation and contains (or points to) `inet_sock` / `tcp_sock`.
- `struct file` -> `struct socket` -> `struct sock`.

---

## 3) `bind()` — what fields are set

User:
```c
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(8080);
addr.sin_addr.s_addr = INADDR_ANY;
bind(s, (struct sockaddr*)&addr, sizeof(addr));
```

What kernel sets in `inet_sock`:
```c
inet->inet_saddr  = addr.sin_addr.s_addr;  /* local IP in network byte order */
inet->inet_sport  = addr.sin_port;         /* local port in network byte order */
inet->inet_daddr  = 0;                     /* not known yet for listening server */
inet->inet_dport  = 0;
```

- `bind()` **reserves** the (local IP, local port) tuple in kernel’s INET/AF_INET tables (so another socket cannot bind the same 4-tuple unless reuse flags permit).
- No remote (destination) fields are filled — remote peer is unknown until connect (client) or accept (server).

---

## 4) `listen()` — accept queue and backlog

User:
```c
listen(s, backlog);
```

Inside kernel:
- `sk->sk_state = TCP_LISTEN;`
- Kernel sets up the `request_sock_queue` (data structure for pending connections)

Representative structure:

```c
struct request_sock {
    struct request_sock *r_next;  /* next in list */
    struct request_sock *r_prev;  /* prev in list */
    __be32 rmt_addr;              /* remote IP */
    __be16 rmt_port;              /* remote port */
    __be32 loc_addr;              /* local IP */
    __be16 loc_port;              /* local port */
    int state;                    /* SYN_RECV, etc. */
    /* SYN cookie and other handshake state fields can be here */
};

struct request_sock_queue {
    struct request_sock *rskq_accept_head; /* head of completed connections */
    struct request_sock *rskq_accept_tail; /* tail of completed connections */
    atomic_t rskq_accept_q_len;            /* count of accepted (completed) */
    spinlock_t rskq_lock;
    /* plus a separate queue for half-open (SYN_RCVD) entries */
};
```

**Backlog semantics**:
- `backlog` limits the number of **completed** (or partly completed, depending on kernel) pending connections.
- Different kernel versions historically interpreted backlog slightly differently (SYN queue vs accept queue thresholds). Recent kernels separate SYN (half-open) and accept (completed) counting.

---

## 5) Connection flow: SYN -> request_sock -> child sock

Timeline:
1. Client sends `SYN` to server `IP:port`.
2. Kernel on server receives SYN and creates a `struct request_sock` (half-open).
   - `request_sock` lives in the SYN queue or half-open queue.
3. Kernel responds with `SYN-ACK`.
4. Client replies with `ACK`.
5. On `ACK`: kernel upgrades the `request_sock` into a full `struct sock` (child socket), or allocates a new `struct sock` and initializes TCP fields from the request.
6. The new `struct sock` is appended to `sk_accept_queue` (accept queue).
7. `accept()` will dequeue from `sk_accept_queue`, allocate a `struct file` in the process `fd` table, and return the `connfd`.

State diagram (simplified):

```
LISTEN socket (sk) : has sk_accept_queue
    |
receive SYN
    |
create request_sock -> half-open (SYN_RCVD)
    |
handshake completes
    |
upgrade to struct sock (child) -> append to sk_accept_queue
    |
accept() pops child, returns new fd
```

---

## 6) `accept()` internals

User:
```c
int connfd = accept(s, NULL, NULL);
```

Kernel work (conceptual):
- If `sk_accept_queue` is empty:
  - If socket is blocking: put process to sleep (wait_event_interruptible or similar) until a connection arrives.
  - If non-blocking: return `-EAGAIN` / `-EWOULDBLOCK`.
- If queue non-empty:
  - Pop child `struct sock` from accept queue.
  - Allocate a `struct file` to represent the new socket in the process's fd table (via `get_unused_fd_flags()`, `fd_install()` etc).
  - Set `file->private_data` to point to the `struct socket`/`struct sock` for this connection.
  - Return fd to userspace. Userspace can now `read()/write()` on the new fd.

Important: the listening socket remains open and continues to accept further connections.

---

## 7) Non-blocking sockets and `O_NONBLOCK`

At userspace:
```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

Inside kernel:
- `file->f_flags` is updated (or underlying `struct socket->flags`/`sk` flags consulted depending on API).
- For sockets, a non-blocking `accept()` will return `-EAGAIN` if no connections are ready.
- For `epoll` usage, sockets are typically set non-blocking before adding to epoll to avoid blocking `read`/`write` while handling multiple fds.

---

## 8) `epoll` — scalable readiness notification

Syscalls:
```c
int efd = epoll_create1(EPOLL_CLOEXEC);
epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev);
epoll_wait(efd, events, maxevents, timeout);
```

Core concepts:
- `epoll` keeps an internal interest list and a ready list.
- `epoll` uses a per-watcher data structure (the `epoll` instance) with links into file/struct's wait queues.
- When a socket becomes readable/writable, the kernel adds it to the epoll instance's ready list (if events match) so `epoll_wait` can return it.

Representative internal structs (conceptual):

```c
struct epitem {
    struct list_head list;      /* linked list node for file -> epoll instances */
    struct file *file;          /* watched file (socket) */
    struct epoll_event ev;      /* event mask user passed */
    /* other handles for multi-epoll watchers */
};

struct epoll_file {
    /* epoll instance root */
    rwlock_t lock;
    struct list_head ready_list; /* ready fds */
    struct rb_root watched;     /* watched fds */
};
```

**How it hooks into sockets**:
- When you `epoll_ctl(ADD)`, kernel installs an `epitem` and attaches wait-queue callbacks to the socket's poll mechanism.
- When the socket's state changes (e.g. data arrives, or a connection completes), the socket's poll callback will notify the epoll instance and add the `epitem` to the ready list (often via `ep_update()` -> `ep_queue_push()`).
- `epoll_wait()` pops ready entries from the ready list and returns to userspace.

**Edge-triggered vs Level-triggered**:
- **Level-triggered (default)**: `epoll_wait` returns a fd while it is readable/writable. If you don't drain the socket, subsequent `epoll_wait` will continue returning that fd immediately.
- **Edge-triggered (EPOLLET)**: `epoll_wait` returns events only when the readiness state **changes** (edge). You **must** fully read/write until `EAGAIN` to avoid losing data.

**Common pattern**:
- Use non-blocking sockets + `EPOLLET` to get high-performance servers.
- On `EPOLLIN` edge event: loop `recv()` until `-1` and `errno == EAGAIN`.

---

## 9) Minimal non-blocking epoll TCP server (C)

```c
/* simplified, error-handling omitted for brevity */
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

int set_nonblocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void){
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(8080), .sin_addr.s_addr = INADDR_ANY };
    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 128);
    set_nonblocking(s);

    int efd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN; /* for server socket, accept events are EPOLLIN */
    ev.data.fd = s;
    epoll_ctl(efd, EPOLL_CTL_ADD, s, &ev);

    struct epoll_event events[64];
    for(;;){
        int n = epoll_wait(efd, events, 64, -1);
        for (int i=0;i<n;i++){
            if (events[i].data.fd == s){
                /* accept all incoming connections */
                while(1){
                    int c = accept(s, NULL, NULL);
                    if (c == -1) break; /* EAGAIN/EWOULDBLOCK when no more */
                    set_nonblocking(c);
                    struct epoll_event cev = { .events = EPOLLIN | EPOLLET, .data.fd = c };
                    epoll_ctl(efd, EPOLL_CTL_ADD, c, &cev);
                }
            } else {
                int fd = events[i].data.fd;
                if (events[i].events & EPOLLIN){
                    /* read until EAGAIN */
                    char buf[4096];
                    while(1){
                        ssize_t r = recv(fd, buf, sizeof(buf), 0);
                        if (r == -1) {
                            if (errno == EAGAIN) break;
                            close(fd); break;
                        }
                        if (r == 0) { close(fd); break; } /* peer closed */
                        /* handle r bytes */
                    }
                }
                /* handle EPOLLOUT etc */
            }
        }
    }
    return 0;
}
```

Notes:
- Setting the listening socket non-blocking avoids blocking on `accept()` when used with edge-triggered mode.
- On accept: you typically accept in a loop until `-1` and `errno == EAGAIN`.

---

## 10) `strace`-style syscall trace (minimal example)

A typical trace for one client connecting and server accepting:

```
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) = 3
bind(3, {sa_family=AF_INET, sin_port=htons(8080), sin_addr=inet_addr("0.0.0.0")}, 16) = 0
listen(3, 128) = 0
accept(3, 0x7ffd..., 0x10) = 4           ; blocks until client connects
read(4, "GET / HTTP/1.1\r\n...", 4096) = 200
write(4, "HTTP/1.1 200 OK\r\n...", 1024) = 1024
close(4) = 0
```

With non-blocking + epoll, you'd see `accept()` return `-1 EAGAIN` when there are no pending connections (instead of blocking).

---

## 11) References & Next steps

- Read Linux kernel networking sources:
  - `net/socket.c`, `net/ipv4/af_inet.c`, `net/ipv4/tcp.c`, `net/ipv4/tcp_ipv4.c`
  - `fs/file.c` (file descriptor internals)
  - `net/core/sock.c` (struct sock utilities)
  - `mm` and `kernel` for wait queues / sleep/wakeup primitives
  - `fs/select.c`, `fs/poll.c`, `kernel/epoll` sources for epoll internals
- Use `pahole` on kernel builds to inspect real layouts.
- Try `strace -f -e trace=network ./server` to see actual syscalls.
- Build a minimal kernel module (or read kernel source) to print actual struct sizes/offsets on your running kernel.

---

## Appendix — Quick lookup of fields (cheat-sheet)

- Listening socket:
  - `struct socket->sk->sk_state == TCP_LISTEN`
  - `sk->sk_accept_queue` holds `request_sock`/child `struct sock`
- Half-open connection:
  - `struct request_sock` stores SYN-handshake temporary fields
- Completed connection:
  - `struct sock` (child) in `sk_accept_queue` -> returned via `accept()`

---

*End of file*
