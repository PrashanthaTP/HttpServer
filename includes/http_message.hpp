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

const std::unordered_map<int, std::string> g_HttpStatusMessage = {{200, "Ok"}};
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

class Response {
   public:
    Response() = default;
    void setStatusCode(HttpStatusCode t_status_code);
    void setContent(const std::string& t_content_str);
    void setHeader(const std::string& key, const std::string& val);
    void parse();
    std::string str() const;
    size_t size() const;

   private:
    HttpStatusCode m_status_code = HttpStatusCode::Ok;
    HttpVersion m_http_version = HttpVersion::HTTP_1_1;
    std::unordered_map<std::string, std::string> m_header_umap;
    std::string m_content_str;
    std::string m_response_str;
};


}  // namespace SimpleHttpServer
