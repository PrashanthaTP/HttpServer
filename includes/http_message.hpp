#pragma once

#include <string>
#include <unordered_map>
//Request and Response

namespace SimpleHttpServer {
enum class HttpMethod {
    HEAD,
    GET,
    POST,
    //...
};

enum class HttpVersion { HTTP_1_1 = 11 };

enum class HttpStatusCode { Ok = 200 };

class Request {
   public:
    Request();
    ~Request() = default;
    std::string header(std::string& key) const;

   private:
    HttpMethod m_http_method;
    std::unordered_map<std::string, std::string> m_headers_umap;
    std::string m_content_str;
};
}  // namespace SimpleHttpServer

class Response {
   public:
    Response() {}
};
