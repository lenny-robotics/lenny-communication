#pragma once

#include <functional>
#include <optional>
#include <string>

namespace lenny::communication {

class UDPServer {
public:
    UDPServer(const int& port);
    ~UDPServer();

    bool open();
    void close();
    bool isOpen() const;
    void loop();

public:
    const int port;

    //We receive `message` from client with `ip` and `port` and optionally send a `response` back
    typedef std::function<void(std::optional<std::string>& response, const std::string& message, const std::string& ip, const std::uint16_t& port)>
        F_messageFromClient;
    F_messageFromClient f_messageFromClient = nullptr;

private:
    bool opened = false;
    int server_fd;
};

}  // namespace lenny::communication