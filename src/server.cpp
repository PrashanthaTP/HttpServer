#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <unistd.h>  //close
#include <cstring>   //memset
#include <iostream>  //cout
#include <thread>

#include "http_message.hpp"
#include "server.hpp"
#include "utils.hpp"

using std::cout;

namespace SimpleHttpServer {
//TODO: replace cout and exist_with_msg with something different
HttpServer::HttpServer(const std::string& t_port_str) : m_port_str(t_port_str) {
    createSocket();
}

HttpServer::~HttpServer() {
    if (!m_is_stopped) {
        stop();
    }
}

void HttpServer::createSocket() {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use IP of the program host

    int stat =
        getaddrinfo(NULL, m_port_str.c_str(), &hints, &m_server_addrinfo_p);
    if (stat != 0) {
        exit_with_msg("Error during getting address");
    }
    m_server_fd =
        socket(m_server_addrinfo_p->ai_family, m_server_addrinfo_p->ai_socktype,
               m_server_addrinfo_p->ai_protocol);
    if (m_server_fd == -1) {
        exit_with_msg("Error during socket creation");
    }
}

void HttpServer::start() {
    int sockopt = 1;
    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockopt,
               sizeof(sockopt));

    if (bind(m_server_fd, m_server_addrinfo_p->ai_addr,
             m_server_addrinfo_p->ai_addrlen) < 0) {
        exit_with_msg("Error during binding socket");
    }
    //print_m_server_addrinfo_p(m_server_addrinfo_p);
    freeaddrinfo(m_server_addrinfo_p);  //should this be moved to destructor

    if (listen(m_server_fd, g_max_backlog) < 0) {
        exit_with_msg("Error during listening");
    } else {
        cout << "Server started listening..\n";
    }
    // acceptConnections();

    setupEpoll();
    setupThreads();
}

void HttpServer::setupEpoll() {
    for (int i = 0; i < g_thread_poolsize; i++) {
        m_epoll_fds[i] = epoll_create1(0);
        if (m_epoll_fds[i] < 0) {
            exit_with_msg("Error creating epoll fd");
        }
    }
}

void HttpServer::setupThreads() {
    m_listener_thread = std::thread(&HttpServer::acceptConnections, this);
    for (int i = 0; i < g_thread_poolsize; i++) {
        m_worker_threads[i] =
            std::thread(&HttpServer::handleConnections, this, i);
    }
}

void HttpServer::acceptConnections() {}

void HttpServer::handleConnections() {}

void HttpServer::closeSocket() {
    if (m_server_fd < 0) {
        // m_server_fd is not holding valid file descriptor
        return;
    } else {
        close(m_server_fd);
    }
}
void HttpServer::closeEpoll() {
    for (int i = 0; i < g_thread_poolsize; i++) {
        if (m_epoll_fds[i] < 0) {
            continue;
        }
        close(m_epoll_fds[i]);
    }
}

void HttpServer::joinThreads() {
    m_listener_thread.join();
    for (int i = 0; i < g_thread_poolsize; i++) {
        m_worker_threads[i].join();
    }
}

void HttpServer::stop() {
    closeSocket();
    closeEpoll();
    joinThreads();
    m_is_stopped = true;
}

}  // namespace SimpleHttpServer
