#pragma once

#include <string>

namespace SimpleHttpServer {
class HttpServer {
   public:
    //TODO: delete certain constructors
    //TODO: use move op for port string initialization
    HttpServer(const std::string& t_port_str);
    ~HttpServer();
    void start();

   private:
    int m_server_fd;
    const std::string m_port_str;
    struct addrinfo* m_server_addrinfo_p;
    bool m_is_running = false;
    void createSocket();
    void acceptConnections();
    void handleClient(int client_fd);
};
}  // namespace SimpleHttpServer
