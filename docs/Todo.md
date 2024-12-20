# Todo

+ Using OOPS / Modern C++ Concepts
+ Chunked file transfer
+ File Caching
+ Logging 
+ Bencmarking


## Refactoring

- [ ] Consistent naming : snake_case?/ camelCase? / cCamelCase?
- [ ] Remove log statements
- [ ] Proper exception handling -> Custom Exceptions to avoid ambiguity
- [ ] Use Smart Pointers
- [ ] Use of std::move

- [✅] Response Handling via objects
- [✅] Request Handling via objects
- [✅] Registering route handles
- [✅] Issue #9: Resend if previous 'send' didn't send all the bytes
- [✅] Cmake
- [ ] Benchmarking
- [ ] Python Client for Testing

## Planning
```bash
HttpServer() -> CreateSocket
|-> Start -> SetSocketOpt -> Bind -> Listen() -> SetupEpoll + CreateThreads

|-> Thread Listener -> AcceptConnections -> accept -> Add to interest list of Epoll FD

|-> Thread Worker -> EpollWait -> EPOLLIN -> recv -> Add to interest list

                             | -> EPOLLOUT -> FormatResponse -> send
```

# Benchmarking

1. SW 1: Thread Per Client
2. SW 2: Thread Pool + EPOLL, Level Triggered, Blocking Sockets
3. SW 3: Thread Pool + EPOLL, Level Triggered, Non Blocking Sockets
