#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  //close
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
    if (m_server_fd < 0) {
        close(m_server_fd);
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

    freeaddrinfo(m_server_addrinfo_p);
    if (listen(m_server_fd, 10) < 0) {
        exit_with_msg("Error during listening");
    }
    acceptConnections();
}

void HttpServer::createSocket() {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use IP of the program host

    if (getaddrinfo(NULL, 
                    m_port_str.c_str(), 
                    &hints, 
                    &m_server_addrinfo_p) != 0) {
        exit_with_msg("Error during getting address");
    }
    m_server_fd =
        socket(m_server_addrinfo_p->ai_family, m_server_addrinfo_p->ai_socktype,
               m_server_addrinfo_p->ai_protocol);
    if (m_server_fd == -1) {
        exit_with_msg("Error during socket creation");
    }
}

void HttpServer::acceptConnections() {
    //client addr storage
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    //accept connections
    //multithreading -> handle clients
    while (1) {
        int client_fd;
        client_fd = accept(m_server_fd, 
                           (struct sockaddr*)&client_addr,
                           &client_addr_size);
        if (client_fd < 1) {
            log_err("Error while accepting client connection");
            continue;
        }
        handleClient(client_fd);
    }
}

void HttpServer::handleClient(int client_fd) {
    char buffer[1024];
    int bytesReceived = recv(client_fd, buffer, sizeof(buffer), 0);
    // if (bytesReceived <= 0) {
    if (bytesReceived < 0) {
        //?
        log_msg("Error while receiving Client msg");
        return;
    }
    cout << buffer << "\n";
    cout << "================\n";
    close(client_fd);
}
}  // namespace SimpleHttpServer
