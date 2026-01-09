#include "WebServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstring>

namespace Banking {

void HttpResponse::setJson(const std::string& json) {
    headers["Content-Type"] = "application/json";
    body = json;
}

void HttpResponse::setHtml(const std::string& html) {
    headers["Content-Type"] = "text/html; charset=utf-8";
    body = html;
}

void HttpResponse::setCss(const std::string& css) {
    headers["Content-Type"] = "text/css; charset=utf-8";
    body = css;
}

void HttpResponse::setJs(const std::string& js) {
    headers["Content-Type"] = "application/javascript; charset=utf-8";
    body = js;
}

void HttpResponse::setNotFound() {
    statusCode = 404;
    statusText = "Not Found";
    setJson("{\"error\": \"Not found\"}");
}

void HttpResponse::setBadRequest(const std::string& message) {
    statusCode = 400;
    statusText = "Bad Request";
    setJson("{\"error\": \"" + message + "\"}");
}

void HttpResponse::setInternalError(const std::string& message) {
    statusCode = 500;
    statusText = "Internal Server Error";
    setJson("{\"error\": \"" + message + "\"}");
}

WebServer::WebServer(int port) : port_(port), serverSocket_(-1), running_(false) {}

WebServer::~WebServer() {
    stop();
}

void WebServer::addRoute(const std::string& method, const std::string& path, RouteHandler handler) {
    routes_[method + " " + path] = handler;
}

void WebServer::setStaticHandler(RouteHandler handler) {
    staticHandler_ = handler;
}

bool WebServer::start() {
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    
    int opt = 1;
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    
    if (bind(serverSocket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind to port " << port_ << "\n";
        close(serverSocket_);
        return false;
    }
    
    if (listen(serverSocket_, 10) < 0) {
        std::cerr << "Failed to listen\n";
        close(serverSocket_);
        return false;
    }
    
    running_ = true;
    serverThread_ = std::thread(&WebServer::serverLoop, this);
    
    std::cout << "Server started on http://localhost:" << port_ << "\n";
    return true;
}

void WebServer::stop() {
    running_ = false;
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
}

bool WebServer::isRunning() const {
    return running_;
}

int WebServer::getPort() const {
    return port_;
}

void WebServer::serverLoop() {
    while (running_) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (running_) {
                std::cerr << "Accept failed\n";
            }
            continue;
        }
        
        handleClient(clientSocket);
        close(clientSocket);
    }
}

void WebServer::handleClient(int clientSocket) {
    char buffer[8192];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string rawRequest(buffer);
    
    HttpRequest request = parseRequest(rawRequest);
    HttpResponse response;
    
    // Look for exact route match
    std::string routeKey = request.method + " " + request.path;
    auto it = routes_.find(routeKey);
    
    if (it != routes_.end()) {
        it->second(request, response);
    } else if (staticHandler_) {
        staticHandler_(request, response);
    } else {
        response.setNotFound();
    }
    
    std::string responseStr = buildResponse(response);
    send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
}

HttpRequest WebServer::parseRequest(const std::string& rawRequest) {
    HttpRequest request;
    std::istringstream stream(rawRequest);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
        // Remove trailing \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        std::istringstream lineStream(line);
        std::string pathWithQuery;
        lineStream >> request.method >> pathWithQuery;
        
        // Parse path and query string
        size_t queryPos = pathWithQuery.find('?');
        if (queryPos != std::string::npos) {
            request.path = pathWithQuery.substr(0, queryPos);
            request.queryParams = parseQueryString(pathWithQuery.substr(queryPos + 1));
        } else {
            request.path = pathWithQuery;
        }
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            // Trim leading space from value
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            request.headers[key] = value;
        }
    }
    
    // Parse body (rest of the request)
    std::ostringstream bodyStream;
    bodyStream << stream.rdbuf();
    request.body = bodyStream.str();
    
    // If body is form data, parse it as query params too
    if (request.headers["Content-Type"] == "application/x-www-form-urlencoded") {
        auto formParams = parseQueryString(request.body);
        for (const auto& [key, value] : formParams) {
            request.queryParams[key] = value;
        }
    }
    
    return request;
}

std::string WebServer::buildResponse(const HttpResponse& response) {
    std::ostringstream stream;
    stream << "HTTP/1.1 " << response.statusCode << " " << response.statusText << "\r\n";
    
    for (const auto& [key, value] : response.headers) {
        stream << key << ": " << value << "\r\n";
    }
    
    stream << "Content-Length: " << response.body.length() << "\r\n";
    stream << "Connection: close\r\n";
    stream << "\r\n";
    stream << response.body;
    
    return stream.str();
}

std::map<std::string, std::string> WebServer::parseQueryString(const std::string& query) {
    std::map<std::string, std::string> params;
    std::istringstream stream(query);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t eqPos = pair.find('=');
        if (eqPos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eqPos));
            std::string value = urlDecode(pair.substr(eqPos + 1));
            params[key] = value;
        }
    }
    
    return params;
}

std::string WebServer::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

} // namespace Banking
