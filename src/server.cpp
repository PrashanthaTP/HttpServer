#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <unistd.h>  //close
#include <algorithm>
#include <cctype>
#include <cstring>   //memset
#include <iostream>  //cout
#include <thread>

#include "http_message.hpp"
#include "server.hpp"
#include "utils.hpp"

using std::cout;

namespace SimpleHttpServer {

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
        throw std::runtime_error("Error during getting server address info");
    }
    m_server_fd = socket(m_server_addrinfo_p->ai_family,
                         m_server_addrinfo_p->ai_socktype | SOCK_NONBLOCK,
                         m_server_addrinfo_p->ai_protocol);
    if (m_server_fd == -1) {
        throw std::runtime_error("Error during server socket creation");
    }
}

void HttpServer::start() {
    int sockopt = 1;
    setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockopt,
               sizeof(sockopt));

    if (bind(m_server_fd, m_server_addrinfo_p->ai_addr,
             m_server_addrinfo_p->ai_addrlen) < 0) {
        throw std::runtime_error("Error during binding server socket");
    }

    freeaddrinfo(m_server_addrinfo_p);  //should this be moved to destructor

    if (listen(m_server_fd, g_max_backlog) < 0) {
        throw std::runtime_error("Error during listening");
    } else {
        cout << "Server started listening on port " << m_port_str << "\n";
    }

    m_is_running = true;
    setupEpoll();
    setupThreads();
}

void HttpServer::setupEpoll() {
    for (int i = 0; i < g_thread_poolsize; i++) {
        m_epoll_fds[i] = epoll_create1(0);
        if (m_epoll_fds[i] < 0) {
            throw std::runtime_error("Error creating epoll fd");
        }
    }
    // log_msg("Setup Epoll FDs successfully\n");
}

void HttpServer::setupThreads() {
    m_listener_thread = std::thread(&HttpServer::acceptConnections, this);
    for (int i = 0; i < g_thread_poolsize; i++) {
        m_worker_threads[i] =
            std::thread(&HttpServer::handleConnections, this, i);
    }
    // log_msg("Threads started successfully\n");
}

void HttpServer::acceptConnections() {
    // log_msg("Inside acceptConnections\n");
    // log_msg("Value of m_is_running " + std::to_string(m_is_running) + "\n");
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    while (m_is_running) {
        //log_msg("Accepting connections\n");
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
    //log_msg("Inside handleConnections\n");
    struct epoll_event ev_list[g_max_events];
    while (m_is_running) {
        //log_msg("Epoll wait..\n");
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

    // cout << "Max bytes that can be read : "
    //      << getMaxSendBytesPossible(request->fd) << " bytes.\n";

    if (n_bytes > 0) {
        response = new EventData();
        response->fd = request->fd;
        request->length = n_bytes;
        createResponse(request, response);
        updateEpoll(epoll_fd, EPOLL_CTL_MOD, request->fd, EPOLLOUT, response);
        delete request;
    } else if (n_bytes == 0) {
        //client closed the connection
        // log_msg("EPOLL_CTL_DEL 1: " + std::to_string(ev->data.fd) + "\n");
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
        // perror("Unknown error in recv");
        // log_msg("EPOLL_CTL_DEL 2: " + std::to_string(ev->data.fd) + "\n");
        updateEpoll(epoll_fd, EPOLL_CTL_DEL, request->fd, 0, nullptr);
        close(request->fd);
        delete request;
    }
}

void HttpServer::handleEpollOut(int epoll_fd, struct epoll_event* ev) {
    EventData* response = static_cast<EventData*>(ev->data.ptr);

    //ssize_t send(int s, const void* buf, size_t len, int flags);
    ssize_t n_bytes = send(response->fd, response->buffer,
                           response->length - response->cursor, 0);
    // cout << "Max bytes that can be sent : "
    //      << getMaxSendBytesPossible(response->fd) << " bytes.\n";

    if (n_bytes == response->length) {
        //successfully sent all data
        //check if client sends some more requests
        struct EventData* request = new EventData();
        request->fd = response->fd;
        updateEpoll(epoll_fd, EPOLL_CTL_MOD, response->fd, EPOLLIN, request);
        delete response;
    } else if (n_bytes > 0) {
        //send remaining data
        response->cursor += n_bytes;
        updateEpoll(epoll_fd, EPOLL_CTL_MOD, response->fd, EPOLLOUT,
                    response);  //is this really required?
    } else if (errno == EAGAIN || EWOULDBLOCK) {
        //retry
        updateEpoll(epoll_fd, EPOLL_CTL_MOD, response->fd, EPOLLOUT,
                    response);  //is this really required?
    } else {
        //unknown error
        updateEpoll(epoll_fd, EPOLL_CTL_DEL, response->fd, 0, nullptr);
        close(response->fd);
        delete response;
    }
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

void HttpServer::registerRouteHandler(std::string route, HttpMethod httpMethod,
                                      RouteHandlerCallback_t callback) {
    std::transform(route.begin(), route.end(), route.begin(),
                   [](char c) -> char { return std::tolower(c); });
    //m_route_handlers_map[route].emplace(httpMethod,callback);//doesn't overwrite
    m_route_handlers_map[route][httpMethod] = callback;
    // for(const std::pair<HttpMethod,RouteHandlerCallback_t> &key : m_route_handlers_map[route]){
    //     log_msg("Path: " + route);
    //     log_msg("Key : " + to_string(key.first));
    // }
}

void HttpServer::createResponse(const EventData* const raw_request,
                                EventData* raw_response) {
    Request request{};
    Response response{};
    try {

        //Populate request object
        memcpy(request.m_buffer, raw_request->buffer, raw_request->length);
        request.parse();
        // Get the user callback for route handling
        std::string path = request.getPath();
        HttpMethod http_method = request.getHttpMethod();

        if (m_route_handlers_map.find(path) == m_route_handlers_map.end()) {
            throw std::domain_error("Resource not found");
        }

        if (m_route_handlers_map.at(path).find(http_method) ==
            m_route_handlers_map.at(path).end()) {
            throw std::invalid_argument("Method not supported");
        }

        // log_msg("No issue with callback register\n");

        RouteHandlerCallback_t callback =
            m_route_handlers_map.at(path).at(http_method);
        //Populate response object using route handler callback
        // Response response{};
        callback(request, response);
    } catch (const std::invalid_argument& e) {  //Todo: Use custom exceptions?
        // Response response{};
        response.setStatusCode(HttpStatusCode::MethodNotAllowed);
        response.setContent(e.what());
        response.parse();
    } catch (const std::domain_error& e) {
        response.setStatusCode(HttpStatusCode::NotFound);
        response.setContent(e.what());
        response.parse();
    } catch (const std::logic_error& e) {
        response.setStatusCode(HttpStatusCode::HttpVersionNotSupported);
        response.setContent(e.what());
        response.parse();
    } catch (const std::exception& e) {
        response.setStatusCode(HttpStatusCode::InternalServerError);
        response.setContent(e.what());
        response.parse();
    }
    //response now has been updated by the api user
    //now we need to get the raw response updated using 'Response' obj
    size_t response_size = response.size();
    raw_response->length = response_size;
    memcpy(raw_response->buffer, response.str().c_str(), response_size);
}

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
    // log_msg("Server Stop Called\n");
    m_is_running = false;
    closeSocket();
    closeEpoll();
    joinThreads();
}

int HttpServer::getMaxReadBytesPossible(int read_fd) {
    int optVal;
    socklen_t optLen;
    int ret = getsockopt(read_fd, SOL_SOCKET, SO_SNDBUF, &optVal, &optLen);
    return ret == 0 ? optVal : ret;
}
int HttpServer::getMaxSendBytesPossible(int send_fd) {
    int optVal;
    socklen_t optLen;
    int ret = getsockopt(send_fd, SOL_SOCKET, SO_RCVBUF, &optVal, &optLen);
    return ret == 0 ? optVal : ret;
}

}  // namespace SimpleHttpServer
