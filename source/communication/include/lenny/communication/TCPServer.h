#pragma once

#include <functional>
#include <string>

namespace lenny::communication {

class TCPServer {
public:
    TCPServer(const int& port);
    ~TCPServer();

    bool open();
    void close();
    bool isOpen() const;
    bool isClientConnected() const;
    void loop();

public:
    const int port;

    typedef std::function<void(std::string& response, const std::string& message)> F_messageFromClient;
    F_messageFromClient f_messageFromClient = nullptr;

private:
    bool opened = false;
    bool clientConnected = false;
    int socket_fd, server_fd;
};

}  // namespace lenny::communication