#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  //close
#include <cstring>   //memset

#include "server.hpp"
#include "utils.hpp"

namespace SimpleHttpServer {
HttpServer::HttpServer() {
    struct addrinfo hints;
    struct addrinfo* server_info;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use IP of the program host

    if (getaddrinfo(NULL, m_port_str, &hints, &server_info) != 0) {
        exitWithMsg("Error during getting address");
    }
    server_fd = socket(server_info->ai_family, server_info->ai_socktype,
                       server_info->ai_protocol);
    if (server_fd == ERR_SOCKET_CREATE) {
        exitWithMsg("Error during socket creation");
    }
    int sockopt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockopt,
               sizeof(sockopt));

    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        exitWithMsg("Error during binding socket");
    }
    //print_server_info(server_info);

    freeaddrinfo(server_info);
}
HttpServer::~HttpServer() {
    if (server_fd < 0) {
        close(server_fd);
    }
}

void HttpServer::listen() {
    if (listen(server_fd, 10) <= ERR_SOCKET_LISTEN) {
        exit_with_msg("Error during listening");
    }
}

}  // namespace SimpleHttpServer
