#pragma once

#include <stdint.h>
#include <string>
#include <thread>

namespace SimpleHttpServer {

const uint16_t g_thread_poolsize = 5;
const uint16_t g_max_backlog = 3;

class HttpServer {
   public:
    //TODO: delete certain constructors
    //TODO: use move op for port string initialization
    HttpServer(const std::string& t_port_str);
    ~HttpServer();
    void start();
    void stop();

   private:
    int m_server_fd;
    const std::string m_port_str;
    struct addrinfo* m_server_addrinfo_p;

    bool m_is_running = false;
    bool m_is_stopped = false;

    int m_epoll_fds[g_thread_poolsize];
    std::thread m_listener_thread;
    std::thread m_worker_threads[g_thread_poolsize];

    void createSocket();
    void closeSocket();

    void acceptConnections();
    void handleConnections();

    void setupEpoll();
    void closeEpoll();

    void setupThreads();
    void joinThreads();
};
}  // namespace SimpleHttpServer
