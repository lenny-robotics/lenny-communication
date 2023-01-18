#include <lenny/communication/TCPClient.h>
#include <lenny/communication/Utils.h>
#include <lenny/tools/Logger.h>

#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace lenny::communication {

TCPClient::TCPClient(const std::string& ip, const int& port) : ip(ip), port(port) {}

TCPClient::~TCPClient() {
    if (isConnected())
        disconnect();
}

bool TCPClient::connect() {
    //Check connection
    if (isConnected())
        return true;

#if WIN32
    //Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        LENNY_LOG_WARNING("Winsock initialization failed")
        return false;
    }
#endif

    //Create socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LENNY_LOG_WARNING("Socket creation failed");
        return false;
    }

    //Convert IPv4 and IPv6 addresses from text to binary form
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        LENNY_LOG_WARNING("Invalid IP address `%s`", ip.c_str());
        return false;
    }

    //Connect
    if ((client_fd = ::connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        LENNY_LOG_WARNING("Connection to server failed");
        return false;
    }

    //Wrap up
    connected = true;
    LENNY_LOG_INFO("TCPClient successfully connected to server with port `%d` and IP `%s", port, ip.c_str());
    return true;
}

void TCPClient::disconnect() {
    //Check connection
    if (!isConnected())
        return;

    //Let server know that connection is closing
    std::optional<std::string> response;
    writeToServer(response, Utils::TCP_CLIENT_CLOSE_MESSAGE, false);

    //Close
#if WIN32
    shutdown(socket_fd, SD_BOTH);
    closesocket(socket_fd);
    shutdown(client_fd, SD_BOTH);
    closesocket(client_fd);
    WSACleanup();
#else
    shutdown(socket_fd, SHUT_RDWR);
    ::close(socket_fd);
    shutdown(client_fd, SHUT_RDWR);
    ::close(client_fd);
#endif

    connected = false;
    LENNY_LOG_INFO("TCPClient successfully disconnected to server on port `%d` and IP `%s", port, ip.c_str());
}

bool TCPClient::isConnected() const {
    return connected;
}

void TCPClient::writeToServer(std::optional<std::string>& response, const std::string& message, const bool& waitForResponse) const {
    //Initialize response
    response = std::nullopt;

    //Perform connection check
    if (!isConnected()) {
        LENNY_LOG_WARNING("Connect to server first before sending messages...")
        return;
    }

    //Perform size check
    if (message.size() > Utils::TCP_BUFFER_SIZE) {
        LENNY_LOG_WARNING("Message that should be sent to server is too long (%d VS %d): `%s`", message.size(), Utils::TCP_BUFFER_SIZE, message.c_str());
        return;
    }

    //Print message
    if (Utils::TCP_PRINT_MESSAGES)
        LENNY_LOG_PRINT(tools::Logger::MAGENTA, "CLIENT (sent to server): `%s`\n", message.c_str());

    //Send message
    const int m_len = send(socket_fd, message.c_str(), message.length(), 0);
    if (m_len == -1) {
        LENNY_LOG_WARNING("Something went wrong when sending message `%s` to server", message.c_str())
        return;
    }

    //Receive response
    if (waitForResponse) {
        char r_buff[Utils::TCP_BUFFER_SIZE];
        const int r_len = recv(socket_fd, r_buff, sizeof(r_buff), 0);
        if (r_len == -1) {
            LENNY_LOG_WARNING("Something went wrong when reading response from server")
            return;
        }
        response = std::string(r_buff).substr(0, r_len);

        //Print response
        if (Utils::TCP_PRINT_MESSAGES)
            LENNY_LOG_PRINT(tools::Logger::MAGENTA, "CLIENT (response from server): `%s`\n", response.value().c_str());
    }
}

}  // namespace lenny::communication