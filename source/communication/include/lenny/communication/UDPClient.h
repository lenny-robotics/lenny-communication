#pragma once

#include <optional>
#include <string>

namespace lenny::communication {

class UDPClient {
public:
    UDPClient(const std::string& ip, const int& port);
    ~UDPClient();

    bool open();
    void close();
    bool isOpen() const;

    void writeToServer(std::optional<std::string>& response, const std::string& message) const;

public:
    const std::string ip;
    const int port;

private:
    bool opened = false;
    int socket_fd;
};

}  // namespace lenny::communication