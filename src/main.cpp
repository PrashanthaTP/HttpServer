#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  //close
#include <iostream>
#include <string>

#include <chrono>
#include <mutex>
#include <thread>

#include "server.hpp"

using std::mutex;
using std::thread;

using SimpleHttpServer::HttpServer;

#define PORT_STR "8080"
#define ERR_SOCKET_CREATE 0
#define ERR_SOCKET_BIND -1
#define ERR_SOCKET_LISTEN -1
#define BUF_READ_SIZE 1024

//TODO : file structure update
void sendFile(int conn_fd, FILE* file);

int exit_with_msg(std::string msg) {
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}

void print_server_info(struct addrinfo* addrinfo) {
    char ipstr[INET6_ADDRSTRLEN];
    std::string ipver;
    void* addr;
    if (addrinfo->ai_family == AF_INET) {
        struct sockaddr_in* ipv4_sockaddr =
            (struct sockaddr_in*)addrinfo->ai_addr;
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

void send404(int conn_fd) {
    std::string not_found_response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 28\r\n"
        "\r\n"
        "<h1>404 Page Not Found</h1>";
    send(conn_fd, not_found_response.c_str(), not_found_response.size(), 0);
}

void handleClient(int conn_fd) {
    mutex mtx;
    char buffer[BUF_READ_SIZE] = {0};
    int bytes_read = recv(conn_fd, buffer, BUF_READ_SIZE, 0);
    if (bytes_read < 0) {
        perror("Failed to read from socket");
        close(conn_fd);
        return;
    }
    //std::cout << "Recieved request : " << buffer << "\n";

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

    if (method == "GET") {
        if (path == "/") {
            path = "pages/index.html";
        } else {
            path = "pages" + path;
        }
        mtx.lock();
        //prevent flushing incomplete message
        std::cout << "Method : " << method << " | Path : " << path << "\n";

        //no need to do c_str() + 1 as leading '/' removed
        FILE* file = fopen(path.c_str(), "r");

        if (file == NULL) {
            send404(conn_fd);
        } else {
            //file exists
            sendFile(conn_fd, file);
        }
    } else if (method == "POST") {
        std::string body = request.substr(request.find("\r\n\r\n") + 4);
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " +
            std::to_string(body.size() + 2) +
            "\r\n"
            "\r\n" +
            body + "\r\n";
        send(conn_fd, response.c_str(), response.size(), 0);
    } else {
        const char* response =
            "HTTP/1.1 200 OK\nContent-Type:text /plain\nContent -Length : "
            "12\n\nHello World\n";
        send(conn_fd, response, strlen(response), 0);
    }
    mtx.unlock();
    close(conn_fd);
}

void sendFile(int conn_fd, FILE* file) {
    fseek(file, 0, SEEK_END);  //steam *, offset, origin
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_content = new char[file_size];
    fread(file_content, sizeof(char), file_size, file);

    std::string response_headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        std::to_string(file_size) +
        "\r\n"
        "\r\n";
    send(conn_fd, response_headers.c_str(), response_headers.size(), 0);
    send(conn_fd, file_content, file_size, 0);
    fclose(file);
    delete[] file_content;
}

int main() {
    HttpServer server("8080");
    try {
        server.start();
        std::cout << "Enter [quit] to stop the server" << std::endl;
        std::string command;
        std::cin >> command;
        while (command != "quit") {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cin >> command;
        }
        //server.stop();
    } catch (const std::exception& e) {
        std::cout << "Exception occured : " << e.what() << "\n";
    }
}
