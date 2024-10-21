#include <netdb.h>   //getaddrinfo, sockets
#include <unistd.h>  //read
#include <iostream>
#include <sstream>
#include <string>

#define PORT_STR "8080"
void exit_with_err(const std::string& err) {
  perror(err.c_str());
  exit(EXIT_FAILURE);
}

int main() {
  struct addrinfo* server_info;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT_STR, &hints, &server_info) != 0) {
    exit_with_err("Error fetching address");
  }
  //create socket
  int server_fd;
  if ((server_fd = socket(server_info->ai_family, server_info->ai_socktype,
                          server_info->ai_protocol)) < 0) {
    exit_with_err("Error creating socket");
  }
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) < 0) {
    exit_with_err("Error while configuring server socket");
  }
  //bind socket
  if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
    exit_with_err("Error while binding socket");
  }

  freeaddrinfo(server_info);

  //listen
  if (listen(server_fd, 10) < 0) {
    exit_with_err("Error while listening");
  }

  std::cout << "Server is listening...\n";
  const std::string response =
      "<html lang='en'><body><h1>Hello There!!</h1></body></html>";

  //accept
  while (true) {
    int conn_fd;
    if ((conn_fd = accept(server_fd, nullptr, nullptr)) < 0) {
      exit_with_err("Error while accepting the client connection");
    }
    char buf[3000];
    read(conn_fd, buf, 3000);
    std::cout << "Received : " << buf << "\n";
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\n";
    ss << "content-type: text/html\n";
    ss << "content-length: " << response.size() << "\n";
    ss << "\n";
    ss << response;
    ss << "\n";
    ss.seekg(0, std::ios::end);
    size_t response_size = ss.tellg();
    ss.seekg(0, std::ios::beg);
    send(conn_fd, ss.str().c_str(), response_size, 0);
    close(conn_fd);
  }
  close(server_fd);
}
