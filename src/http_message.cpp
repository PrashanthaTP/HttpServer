#include "http_message.hpp"
#include <sstream>
// Request and Response

namespace SimpleHttpServer {
Request::Request() : m_http_method(HttpMethod::GET) {}
std::string Request::header(std::string& key) const {
    if (m_headers_umap.count(key) > 1) {
        return m_headers_umap.at(key);
    }
    return std::string();
}
std::string to_string(HttpVersion http_version) {
    switch (http_version) {
        case HttpVersion::HTTP_1_1:
            return "HTTP/1.1";
        default:
            //throw error?
            return "";
    }
}
int to_int(HttpStatusCode status_code) {
    switch (status_code) {
        case HttpStatusCode::Ok:
            return 200;
        default:
            //throw error?
            return 0;
    }
}
void Response::setStatusCode(HttpStatusCode t_status_code) {
    m_status_code = t_status_code;
}

void Response::setHeader(const std::string& key, const std::string& val) {
    m_header_umap[key] = val;
}
void Response::setContent(const std::string& t_content_str) {
    m_content_str = t_content_str;
}
void Response::parse() {
    std::ostringstream oss;
    oss << to_string(m_http_version) << " ";
    oss << to_int(m_status_code) << " ";
    oss << g_HttpStatusMessage.at(to_int(m_status_code)) << "\r\n";
    for (const auto& kv : m_header_umap) {
        oss << kv.first << ": " << kv.second << "\r\n";
    }
    oss << "Content-Length:" << m_content_str.size() << "\r\n";
    oss << "\r\n";
    oss << m_content_str;
    m_response_str = oss.str();
    //return m_response_str;
}
std::string Response::str() const {
    return m_response_str;
}
size_t Response::size() const {
    return m_response_str.size();
}
}  // namespace SimpleHttpServer
