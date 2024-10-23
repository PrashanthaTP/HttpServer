#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  //close
#include <iostream>
#include <string>

#define PORT_STR "8080"
#define ERR_SOCKET_CREATE 0
#define ERR_SOCKET_BIND -1
#define ERR_SOCKET_LISTEN -1
#define BUF_READ_SIZE 1024
int exit_with_msg(std::string msg) {
  perror(msg.c_str());
  exit(EXIT_FAILURE);
}

void print_server_info(struct addrinfo* addrinfo) {
  char ipstr[INET6_ADDRSTRLEN];
  std::string ipver;
  void* addr;
  if (addrinfo->ai_family == AF_INET) {
    struct sockaddr_in* ipv4_sockaddr = (struct sockaddr_in*)addrinfo->ai_addr;
    addr = &(ipv4_sockaddr->sin_addr);
    ipver = "IPv4";
  } else {  //AF_INET6
    struct sockaddr_in6* ipv6_sockaddr =
        (struct sockaddr_in6*)addrinfo->ai_addr;
    addr = &(ipv6_sockaddr->sin6_addr);
    ipver = "IPv6";
  }

  inet_ntop(addrinfo->ai_family, addr, ipstr, sizeof(ipstr));
  std::cout << "Server Info: " << "(" << ipver << ") " << ipstr << "\n";
}

void print_server_start_msg() {
  std::cout << "Server listening\n";
}

int main() {
  struct addrinfo hints;
  struct addrinfo* server_info;
  int socket_fd, conn_fd;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;  // use IP of the program host

  if (getaddrinfo(NULL, "8080", &hints, &server_info) != 0) {
    exit_with_msg("Error during getting address");
  }
  socket_fd = socket(server_info->ai_family, server_info->ai_socktype,
                     server_info->ai_protocol);
  if (socket_fd == ERR_SOCKET_CREATE) {
    exit_with_msg("Error during socket creation");
  }
  int sockopt = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sockopt,
             sizeof(sockopt));

  if (bind(socket_fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
    exit_with_msg("Error during binding socket");
  }
  print_server_info(server_info);

  freeaddrinfo(server_info);

  if (listen(socket_fd, 10) <= ERR_SOCKET_LISTEN) {
    exit_with_msg("Error during listening");
  }
  print_server_start_msg();

  while (true) {
    if ((conn_fd = accept(socket_fd, nullptr, nullptr)) < 0) {
      exit_with_msg("Error while accepting connection");
    }
    std::cout << "connection accepted\n";
    char buffer[BUF_READ_SIZE] = {0};
    int bytes_read = recv(conn_fd, buffer, BUF_READ_SIZE, 0);
    if (bytes_read < 0) {
      perror("Failed to read from socket");
      close(conn_fd);
      continue;
    }
    std::cout << "Recieved request : " << buffer << "\n";

    // extract method and path
    /*
    GET / HTTP/1.1
    Host: localhost:8080
    User-Agent: curl/8.9.0
    Accept: 
    */
    std::string request(buffer);
    size_t method_end = request.find(' ');
    std::string method = request.substr(0, method_end);

    size_t path_start = method_end + 1;
    size_t path_end = request.find(' ', path_start);
    std::string path = request.substr(path_start, path_end - path_start);

    std::cout << "Method : " << method << " | Path : " << path << "\n";

    const char* response =
        "HTTP/1.1 200 OK\nContent-Type:text /plain\nContent -Length : "
        "12\n\nHello World\n";
    send(conn_fd, response, strlen(response), 0);
    close(conn_fd);
  }

  close(socket_fd);
}
