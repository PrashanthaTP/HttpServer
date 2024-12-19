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

enum class HttpStatusCode {
    Ok = 200,
    BadRequest = 400,
    NotFound = 404,
    MethodNotAllowed = 405,
    RequestTimeout = 408,
    InternalServerError = 500,
    NotImplemented = 501,
    ServiceUnavailable = 503,
    HttpVersionNotSupported = 505
};
const std::unordered_map<int, std::string> g_HttpStatusMap = {
    {200, "Ok"},
    {400, "BadRequest"},
    {404, "NotFound"},
    {405, "MethodNotAllowed"},
    {408, "RequestTimeout"},
    {500, "InternalServerError"},
    {501, "NotImplemented"},
    {503, "ServiceUnavailable"},
    {505, "HttpVersionNotSupported"}};

const std::unordered_map<std::string, HttpVersion> g_HttpVersionMap = {
    {"HTTP/1.1", HttpVersion::HTTP_1_1}};
const std::unordered_map<std::string, HttpMethod> g_HttpMethodMap = {
    {"GET", HttpMethod::GET}};

std::string to_string(HttpMethod http_method);
std::string to_string(HttpVersion http_version);
HttpMethod string_to_http_method(const std::string& http_method);
HttpVersion string_to_http_version(const std::string& http_version);
int to_int(HttpStatusCode status_code);

class EventData {
   public:
    int fd = -1;
    int length = 0;
    int cursor = 0;
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
    std::string getPath();
    HttpMethod getHttpMethod();
    HttpVersion getHttpVersion();

   private:
    HttpVersion m_http_version;
    HttpMethod m_http_method;
    std::string m_path;
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
    //TODO: should we parse automatically in the constructor itself? get buffer as an arg?

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
