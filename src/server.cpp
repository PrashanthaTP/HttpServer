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
    if (m_is_running) {
        stop();
    } else {
        closeSocket();
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
    m_server_fd = socket(m_server_addrinfo_p->ai_family,
                         m_server_addrinfo_p->ai_socktype | SOCK_NONBLOCK,
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
    m_is_running = true;
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
    log_msg("Setup Epoll FDs successfully\n");
}

void HttpServer::setupThreads() {
    m_listener_thread = std::thread(&HttpServer::acceptConnections, this);
    for (int i = 0; i < g_thread_poolsize; i++) {
        m_worker_threads[i] =
            std::thread(&HttpServer::handleConnections, this, i);
    }
    log_msg("Threads started successfully\n");
}

void HttpServer::acceptConnections() {
    log_msg("Inside acceptConnections\n");
    log_msg("Value of m_is_running " + std::to_string(m_is_running));
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    while (m_is_running) {
        log_msg("Accepting connections\n");
        //TODO: non blocking sockets?
        //https://stackoverflow.com/questions/26269448/why-is-non-blocking-sockets-recommended-in-epoll
        int client_fd = accept4(m_server_fd, (struct sockaddr*)&client_addr,
                                &client_addr_size, SOCK_NONBLOCK);
        if (client_fd < 0) {
            continue;
        }
        //add to interest list of one of the epoll fds
        EventData* event_data = new EventData;
        event_data->fd = client_fd;
        updateEpoll(m_epoll_fds[m_curr_worker_idx], EPOLL_CTL_ADD, client_fd,
                    EPOLLIN, event_data);
        m_curr_worker_idx = (m_curr_worker_idx + 1) % g_thread_poolsize;
    }
}

void HttpServer::handleConnections(int worker_idx) {
    log_msg("Inside handleConnections\n");
    struct epoll_event ev_list[g_max_events];
    while (m_is_running) {
        log_msg("Epoll wait..\n");
        //https://stackoverflow.com/a/62967588/12988588
        int n_fds =
            epoll_wait(m_epoll_fds[worker_idx], ev_list, g_max_events, 0);
        if (n_fds <= 0) {
            continue;
        }
        //TODO:
        //From the book The Linux Programming Interface (Page 1363):
        //if EPOLLIN and EPOLLHUP were both set, then there might
        //be more than MAX_BUF bytes to read.
        for (int i = 0; i < n_fds; i++) {
            struct epoll_event* curr_ev = &ev_list[i];
            struct EventData* ev_data =
                static_cast<EventData*>(curr_ev->data.ptr);
            if (curr_ev->events & EPOLLERR || curr_ev->events & EPOLLHUP) {
                //TODO: Should this block be moved to seperate function?
                updateEpoll(m_epoll_fds[worker_idx], EPOLL_CTL_DEL, ev_data->fd,
                            0, nullptr);
                delete ev_data;
                close(ev_data->fd);
            } else if (curr_ev->events & EPOLLIN) {
                handleEpollIn(m_epoll_fds[worker_idx], curr_ev);
            } else if (curr_ev->events & EPOLLOUT) {
                handleEpollOut(m_epoll_fds[worker_idx], curr_ev);
            }
        }
        //TODO maybe draw diagram connecting the reference code
    }
}
void HttpServer::handleEpollIn(int epoll_fd, struct epoll_event* ev) {
    EventData* request = static_cast<EventData*>(ev->data.ptr);
    EventData* response;
    //ssize_t recv(int s, void *buf, size_t len, int flags);
    ssize_t n_bytes = recv(request->fd, request->buffer, g_max_buffer_size, 0);
    if (n_bytes > 0) {
        response = new EventData();
        response->fd = request->fd;
        createResponse(request, response);
        updateEpoll(epoll_fd, EPOLL_CTL_MOD, request->fd, EPOLLOUT, response);
        delete request;
    } else if (n_bytes == 0) {
        //client closed the connection
        log_msg("EPOLL_CTL_DEL 1: " + std::to_string(ev->data.fd) + "\n");
        updateEpoll(epoll_fd, EPOLL_CTL_DEL, request->fd, 0, nullptr);
        close(request->fd);
        delete request;
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //resource busy or no data
        //try again
        //update epoll interest list with new data for 'fd'
        updateEpoll(epoll_fd, EPOLL_CTL_MOD, request->fd, EPOLLIN, request);
    } else {
        //unexpected error n_bytes<0
        perror("Unknown error in recv");
        log_msg("EPOLL_CTL_DEL 2: " + std::to_string(ev->data.fd) + "\n");
        updateEpoll(epoll_fd, EPOLL_CTL_DEL, request->fd, 0, nullptr);
        close(request->fd);
        delete request;
    }
}

void HttpServer::handleEpollOut(int epoll_fd, struct epoll_event* ev) {
    EventData* response = static_cast<EventData*>(ev->data.ptr);
    std::string body = "C++ is Cool";
    std::string response_str =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " +
        std::to_string(body.size() + 2) +
        "\r\n"
        "\r\n" +
        body + "\r\n";
    //ssize_t send(int s, const void* buf, size_t len, int flags);
    ssize_t n_bytes =
        send(response->fd, response_str.c_str(), response_str.size(), 0);
    if (n_bytes < 0) {
        log_msg("fd from ev.data: " + std::to_string(ev->data.fd) + "\n");
        log_msg("fd from ev.data.ptr: " + std::to_string(response->fd) + "\n");
        perror("Error sending response");
        exit_with_msg("Error sending response");
    }

    log_msg("EPOLL_CTL_DEL 3: " + std::to_string(ev->data.fd) + "\n");
    updateEpoll(epoll_fd, EPOLL_CTL_DEL, response->fd, 0, nullptr);
    close(response->fd);
    delete response;
}

void HttpServer::updateEpoll(int epoll_fd, int op, int fd,
                             uint32_t event_bitmask, void* data) {
    switch (op) {
        case EPOLL_CTL_ADD:
        case EPOLL_CTL_MOD:
            struct epoll_event event;
            event.events = event_bitmask;
            //event.data.fd = fd;//event.data is a union
            event.data.ptr = data;
            if (epoll_ctl(epoll_fd, op, fd, &event) < 0) {
                perror("Error updating interest list");
                throw std::runtime_error("Error updating epoll interest list");
            }
            break;
        case EPOLL_CTL_DEL:
            if (epoll_ctl(epoll_fd, op, fd, nullptr) < 0) {
                perror("Error deleting fd from interest list");
                throw std::runtime_error(
                    "Error deleting fd from epoll interest list");
            }
    }
}

void HttpServer::createResponse(const EventData* const request,
                                EventData* response) {}

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
    log_msg("Server Stop Called\n");
    m_is_running = false;
    closeSocket();
    closeEpoll();
    joinThreads();
}

}  // namespace SimpleHttpServer
