#include <iostream>
#include <string>
#include <chrono>

#include "server.hpp"

using SimpleHttpServer::HttpMethod;
using SimpleHttpServer::HttpServer;
using SimpleHttpServer::HttpStatusCode;
using SimpleHttpServer::Request;
using SimpleHttpServer::Response;
using SimpleHttpServer::RouteHandlerCallback_t;

void registerRouteHandlers(HttpServer& server) {
    RouteHandlerCallback_t rootHandler = [](const Request& request,
                                            Response& response) {
        std::string body = "C++ is Cool\n";
        response.setStatusCode(HttpStatusCode::Ok);
        response.setHeader("Content-Type", "text/html");
        response.setContent(body);
        response.parse();
    };

    server.registerRouteHandler("/", HttpMethod::GET, rootHandler);
}

int main() {
    HttpServer server("8080");
    registerRouteHandlers(server);
    try {
        server.start();
        std::cout << "Enter [quit] to stop the server\n";
        std::string command;
        std::cin >> command;
        while (command != "quit") {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cin >> command;
        }
        server.stop();
    } catch (const std::exception& e) {
        std::cout << "Exception occured : " << e.what() << "\n";
    }
}
