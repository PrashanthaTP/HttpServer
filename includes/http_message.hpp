#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
//Request and Response

namespace SimpleHttpServer {

const uint32_t g_max_buffer_size = 1024;

enum class HttpMethod {
    HEAD,
    GET,
    POST,
    //...
};

enum class HttpVersion { HTTP_1_1 = 11 };

enum class HttpStatusCode { Ok = 200 };
const std::unordered_map<int, std::string> g_HttpStatusMessageMap = {
    {200, "Ok"}};
const std::unordered_map<std::string, HttpVersion> g_HttpVersionMap = {
    {"HTTP/1.1", HttpVersion::HTTP_1_1}};
const std::unordered_map<std::string, HttpMethod> g_HttpMethodMap = {
    {"GET", HttpMethod::GET}};

class EventData {
    public:
    int fd = -1;
    uint32_t length = 0;
    char buffer[g_max_buffer_size];
};

class Request {
   private:
    static const unsigned int g_MaxBufferSize = 1024;

   public:
    char m_buffer[g_MaxBufferSize];
    Request();
    ~Request() = default;
    std::string getHeader(std::string& key) const;
    void parse();

   private:
    HttpVersion m_http_version;
    HttpMethod m_http_method;
    std::unordered_map<std::string, std::string> m_headers_umap;
    std::string m_content_str;
};  // class Request

class Response {
   public:
    Response() = default;
    ~Response() = default;
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
};  //class Response

}  // namespace SimpleHttpServer
