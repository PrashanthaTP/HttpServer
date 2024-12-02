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
    } else {
        cout << "Server started listening..\n";
    }
    acceptConnections();
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

// int countThread = 0;
void HttpServer::acceptConnections() {
    //client addr storage
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    //accept connections
    //multithreading -> handle clients
    while (1) {
        int client_fd;
        client_fd = accept(m_server_fd, (struct sockaddr*)&client_addr,
                           &client_addr_size);
        if (client_fd < 1) {
            log_err("Error while accepting client connection");
            continue;
        }
        std::thread client_thread(&HttpServer::handleClient, this, client_fd);
        client_thread.detach();
    }
}
int checkReceiveError(int bytes_received) {
    if (bytes_received < 0) {
        //?
        log_msg("Error while receiving Client msg");
        return -1;
    } else if (bytes_received == 0) {
        log_msg("Client has closed the connection");
        return -1;
    } else {
    }
    return 0;
}

void HttpServer::handleClient(int client_fd) {
    // countThread++;
    // cout << "Thread: " << countThread << "\n";
    Request request;
    int bytes_received =
        recv(client_fd, request.m_buffer, sizeof(request.m_buffer), 0);
    // if (bytesReceived <= 0) {
    if (checkReceiveError(bytes_received) < 0) {
        close(client_fd);
        return;
    }
    cout << "Incoming request : \n";
    cout << request.m_buffer << "\n";
    cout << "--------------------\n";
    request.parse();
    Response response;
    response.setStatusCode(HttpStatusCode::Ok);
    response.setHeader("Content-Type", "text/html");
    response.setContent("<h1>Hello World</h1>\r\n");
    // response.setContent("<h1>Hello World " + std::to_string(countThread) + "</h1>\r\n");
    response.parse();
    //what if `send` doesn't send the whole buffer?
    //what to be done on partial transfer?
    //anyway for current use case it almost always sends the whole message
    //considering the message size
    send(client_fd, response.str().c_str(), response.size(), 0);
    cout << response.str() << "\n";
    cout << "Response sent\n";
    cout << "--------------------\n";
    //close(client_fd);
}

}  // namespace SimpleHttpServer
