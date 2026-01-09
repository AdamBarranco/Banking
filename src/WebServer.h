#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <atomic>

namespace Banking {

struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> queryParams;
};

struct HttpResponse {
    int statusCode = 200;
    std::string statusText = "OK";
    std::map<std::string, std::string> headers;
    std::string body;
    
    void setJson(const std::string& json);
    void setHtml(const std::string& html);
    void setCss(const std::string& css);
    void setJs(const std::string& js);
    void setNotFound();
    void setBadRequest(const std::string& message);
    void setInternalError(const std::string& message);
};

class WebServer {
public:
    using RouteHandler = std::function<void(const HttpRequest&, HttpResponse&)>;
    
    explicit WebServer(int port = 8080);
    ~WebServer();
    
    void addRoute(const std::string& method, const std::string& path, RouteHandler handler);
    void setStaticHandler(RouteHandler handler);
    
    bool start();
    void stop();
    bool isRunning() const;
    int getPort() const;

private:
    int port_;
    int serverSocket_;
    std::atomic<bool> running_;
    std::thread serverThread_;
    
    std::map<std::string, RouteHandler> routes_;
    RouteHandler staticHandler_;
    
    void serverLoop();
    void handleClient(int clientSocket);
    HttpRequest parseRequest(const std::string& rawRequest);
    std::string buildResponse(const HttpResponse& response);
    std::map<std::string, std::string> parseQueryString(const std::string& query);
    std::string urlDecode(const std::string& str);
};

} // namespace Banking

#endif // WEBSERVER_H
