# Http Server in C++

> Network Programming


## Build

```bash
g++ -o build/server server.cpp
```

## Run

To run Server
```bash
./build/server
```

To connect to server
```
curl http://localhost:8080
```

## 

1. Why thread per client doesnt scale

A "thread per client" approach doesn't scale well because it can quickly lead to excessive resource consumption, particularly memory, when dealing with a large number of concurrent clients, as each client requires its own thread, which can overwhelm the system, especially if many clients are waiting for network operations, leading to inefficient utilization of CPU cores and potential performance degradation; this is often referred to as the "C10K problem" in the context of web servers. 
Key reasons why thread per client doesn't scale:

+ Memory overhead:
Each thread needs its own stack space in memory, which can quickly add up when dealing with a large number of clients, potentially causing memory exhaustion. 
+ Context switching overhead:
Frequent switching between many threads can become a performance bottleneck, as the operating system needs to save and restore the state of each thread. 
+ Limited CPU cores:
Even with multiple CPU cores, a high number of threads can lead to excessive context switching overhead, as the CPU needs to rapidly switch between them, limiting the actual processing power available for each client. 

Alternative approaches for better scalability:
- Thread pool:
A limited number of worker threads are maintained in a pool, where tasks are queued and processed by available threads, reducing the overhead of creating and destroying threads. 
- Asynchronous I/O:
Utilizing non-blocking I/O mechanisms allows the server to handle multiple clients concurrently without creating a separate thread for each one, improving overall responsiveness. 
- Event-driven programming:
This approach relies on events to trigger processing, enabling efficient handling of a large number of concurrent clients with fewer threads
