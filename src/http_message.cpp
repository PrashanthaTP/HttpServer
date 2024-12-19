#include "http_message.hpp"
#include <sstream>
// Request and Response
#include "utils.hpp"

namespace SimpleHttpServer {
Request::Request() : m_http_method(HttpMethod::GET) {}
std::string Request::getHeader(std::string& key) const {
    if (m_headers_umap.count(key) > 1) {
        return m_headers_umap.at(key);
    }
    return std::string();
}

HttpMethod Request::getHttpMethod() {
    return m_http_method;
}

HttpVersion Request::getHttpVersion() {
    return m_http_version;
}

std::string Request::getPath() {
    return m_path;
}

void Request::parse() {
    std::istringstream request_stream{m_buffer};
    std::string http_method, path, http_version;
    request_stream >> http_method >> path >> http_version;
    m_http_method = string_to_http_method(http_method);
    m_http_version = string_to_http_version(http_version);
    m_path = path;

    // log_msg("===========================\n");
    // log_msg("Request :\n");
    // log_msg("Method: ");
    // log_msg(to_string(m_http_method));
    // log_msg("\n");
    // log_msg("Path: ");
    // log_msg(m_path);
    // log_msg("\n");
    // log_msg("Version: ");
    // log_msg(to_string(m_http_version));
    // log_msg("\n");
    // log_msg("===========================\n");
}

std::string to_string(HttpMethod http_method) {
    switch (http_method) {
        case HttpMethod::GET:
            return "GET";
        default:
            //throw error?
            return "";
    }
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
HttpMethod string_to_http_method(const std::string& http_method) {
    if (g_HttpMethodMap.find(http_method) != g_HttpMethodMap.end()) {
        return g_HttpMethodMap.at(http_method);
    }
    throw std::invalid_argument("Http Method ( " + http_method + " ) Not Supported");
}

HttpVersion string_to_http_version(const std::string& http_version) {
    if (g_HttpVersionMap.find(http_version) != g_HttpVersionMap.end()) {
        return g_HttpVersionMap.at(http_version);
    }
    throw std::logic_error("Http Version ( " + http_version + " ) Not Supported");
}

int to_int(HttpStatusCode status_code) {
    switch (status_code) {
        case HttpStatusCode::Ok:
            return 200;
        case HttpStatusCode::BadRequest:
            return 400;
        case HttpStatusCode::NotFound:
            return 404;
        case HttpStatusCode::MethodNotAllowed:
            return 405;
        case HttpStatusCode::RequestTimeout:
            return 408;
        case HttpStatusCode::InternalServerError:
            return 500;
        case HttpStatusCode::NotImplemented:
            return 501;
        case HttpStatusCode::ServiceUnavailable:
            return 503;
        case HttpStatusCode::HttpVersionNotSupported:
            return 505;
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
    oss << g_HttpStatusMap.at(to_int(m_status_code)) << "\r\n";
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
