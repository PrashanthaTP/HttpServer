#pragma once

#include <string>

namespace SimpleHttpServer {
class HttpServer {
   public:
    HttpServer(const std::string& port) {}
    ~HttpServer() {}

    void listen();

   private:
    int m_server_fd_int;
    const string m_port_str
};
}  // namespace SimpleHttpServer
