#include "http_message.hpp"
// Request and Response

namespace SimpleHttpServer {
Request::Request() : m_http_method(HttpMethod::GET) {}
std::string Request::header(std::string& key) const {
    if (m_headers_umap.count(key) > 1) {
        return m_headers_umap.at(key);
    }
    return std::string();
}
}  // namespace SimpleHttpServer


