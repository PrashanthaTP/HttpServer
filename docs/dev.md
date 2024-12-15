# Hints for development

## Socket Programming in Linux

* Header for `struct addrinfo` : `netdb.h`
  - Hint : [Github issue: Incomplete type error on struct addrinfo #2025](https://github.com/Microsoft/vscode-cpptools/issues/2025)
* Writing http response : [Writing proper HTTP response to plain / naked socket connection#1261](https://github.com/nodejs/help/issues/1261)
* Discussion about bind: [socket bind always returns 0, why?](https://www.thecodingforums.com/threads/socket-bind-always-returns-0-why.971444/)
* [libevent: API -> Mechanism to handle callbacks when specific event happens on a file descriptor](https://libevent.org/libevent-book/Ref10_http_server.html)
* [IBM Article: EAGAIN, EWOULDBLOCK](https://www.ibm.com/support/pages/why-does-send-return-eagain-ewouldblock)
* [Stackoverflow: Why is non-blocking sockets recommended in epoll](https://stackoverflow.com/questions/26269448/why-is-non-blocking-sockets-recommended-in-epoll)
- Related 
  1. [Section Level-triggered and edge-triggered in man7/epoll](https://man7.org/linux/man-pages/man7/epoll.7.html)
  2. [Blocking I/O, Nonblocking I/O, And Epoll](https://eklitzke.org/blocking-io-nonblocking-io-and-epoll)
## Cpp

+ [Modern C++ Features](https://github.com/AnthonyCalandra/modern-cpp-features?)
+ [Malloc vs New for Primitives](https://stackoverflow.com/questions/44588345/malloc-vs-new-for-primitives)
+ Fascinating
  - [Placement new:  new(&x)A();](https://stackoverflow.com/a/56896234/12988588)
  - [Object Slicing Mentioned here - Stackoverflow - how-to-throw-a-c-exception ](https://stackoverflow.com/a/52402941)
+ [Cpp Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-conventional)

##  Multithreading

+ [Stackoverflow: What exactly is std::atomic](https://stackoverflow.com/questions/31978324/what-exactly-is-stdatomic)
+ [Stackoverflow: lock vs try lock sleep repeat performance](https://stackoverflow.com/questions/33046900/lock-vs-try-lock-sleep-repeat-performance)
   -  [Wikipedia: Spinlock](https://en.wikipedia.org/wiki/Spinlock)
+ [Reddit: Creating thread inside a class method](https://www.reddit.com/r/learnprogramming/comments/qfuxe6/c_using_threads_within_a_class_method/)

## Reference Projects

+ [Blog by Trung Vuong Thien: A simple HTTP server from scratch](https://trungams.github.io/2020-08-23-a-simple-http-server-from-scratch/)
  - [Github: trungams/http-server](https://github.com/trungams/http-server/blob/master/src/main.cc)
  - [Linux Docs: epoll](https://man7.org/linux/man-pages/man7/epoll.7.html)

## C++ Style Guide

+ [lefticus.gitbooks.io/cpp-best-practices](https://lefticus.gitbooks.io/cpp-best-practices/content/03-Style.html)

## General

+ [Stackoverflow: Running curl command in parallel](https://stackoverflow.com/questions/46362284/run-multiple-curl-commands-in-parallel)


## Web Servers

+ [c10k Problem](http://www.kegel.com/c10k.html#strategies)
