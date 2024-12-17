#pragma once

#include <cstdint>
#include <string>
#include <thread>

#include "http_message.hpp"

namespace SimpleHttpServer {
using RouteHandlerCallback_t = void (*)(const Request& request,
                                        Response response);
const uint16_t g_thread_poolsize = 5;
const uint16_t g_max_backlog = 3;
const uint16_t g_max_connections = 10000;
const uint16_t g_max_events = 10000;

class HttpServer {
   public:
    //TODO: delete certain constructors
    //TODO: use move op for port string initialization
    HttpServer(const std::string& t_port_str);
    ~HttpServer();
    void start();
    void stop();
    void registerRouteHandler(const std::string route, HttpMethod httpMethod,
                              RouteHandlerCallback_t callback);

   private:
    int m_server_fd;
    const std::string m_port_str;
    struct addrinfo* m_server_addrinfo_p;

    std::unordered_map<std::string,
                       std::unordered_map<HttpMethod, RouteHandlerCallback_t>>
        m_route_handlers_map;

    //used in worker threads to check if we can still accept new connections
    bool m_is_running = false;
    // bool m_is_stopped = false;//used in destructor call

    int m_epoll_fds[g_thread_poolsize];
    std::thread m_listener_thread;
    std::thread m_worker_threads[g_thread_poolsize];
    uint16_t m_curr_worker_idx = 0;

    void createSocket();
    void closeSocket();

    void acceptConnections();
    void handleConnections(int worker_idx);

    void setupEpoll();
    void updateEpoll(int epoll_fd, int op, int fd, uint32_t event_bitmask,
                     void* data);
    //TODO: pass only EventData as second arg for handleEpollX functions?
    void handleEpollIn(int epoll_fd, struct epoll_event* ev);
    void handleEpollOut(int epoll_fd, struct epoll_event* ev);
    void closeEpoll();

    void createResponse(const EventData* const raw_request,
                        EventData* raw_response);

    void setupThreads();
    void joinThreads();
};
}  // namespace SimpleHttpServer
