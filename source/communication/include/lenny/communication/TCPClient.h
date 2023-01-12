#pragma once

#include <optional>
#include <string>

namespace lenny::communication {

class TCPClient {
public:
    TCPClient(const std::string& ip, const int& port);
    ~TCPClient();

    bool connect();
    void disconnect();
    bool isConnected() const;

    void writeToServer(std::optional<std::string>& response, const std::string& message) const;

public:
    const std::string ip;
    const int port;

private:
    bool connected = false;
    int socket_fd, client_fd;
};

}  // namespace lenny::communication