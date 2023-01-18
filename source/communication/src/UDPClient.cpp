#include <lenny/communication/UDPClient.h>
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

UDPClient::UDPClient(const std::string& ip, const int& port) : ip(ip), port(port) {}

UDPClient::~UDPClient() {
    if (isOpen())
        close();
}

bool UDPClient::open() {
    //Check connection
    if (isOpen())
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
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LENNY_LOG_WARNING("Socket creation failed")
        return false;
    }

    //Wrap up
    opened = true;
    LENNY_LOG_INFO("UDPClient successfully opened with ip `%s` and port `%d`", ip.c_str(), port)
    return true;
}

void UDPClient::close() {
    //Check connection
    if (!isOpen())
        return;

//Close
#if WIN32
    shutdown(socket_fd, SD_BOTH);
    closesocket(socket_fd);
    WSACleanup();
#else
    shutdown(socket_fd, SHUT_RDWR);
    ::close(socket_fd);
#endif
    opened = false;
    LENNY_LOG_INFO("UDPClient successfully closed with ip `%s` and port `%d`", ip.c_str(), port)
}

bool UDPClient::isOpen() const {
    return opened;
}

void UDPClient::writeToServer(std::optional<std::string>& response, const std::string& message, const bool& waitForResponse) const {
    //Initialize response
    response = std::nullopt;

    //Perform checks
    if (!isOpen()) {
        LENNY_LOG_WARNING("Open socket first before sending messages...")
        return;
    }

    if (message.size() > Utils::UDP_BUFFER_SIZE) {
        LENNY_LOG_WARNING("Message that should be sent to server is too long (%d VS %d): `%s`", message.size(), Utils::UDP_BUFFER_SIZE, message.c_str())
        return;
    }

    //Print message
    if (Utils::UDP_PRINT_MESSAGES)
        LENNY_LOG_PRINT(tools::Logger::MAGENTA, "CLIENT (sent to server): `%s`\n", message.c_str())

    //Gather address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        LENNY_LOG_WARNING("Invalid IP address `%s`", ip.c_str())
        return;
    }
    socklen_t len = sizeof(serv_addr);

    //Send message
    const int m_len = sendto(socket_fd, message.c_str(), message.size(), 0, (const struct sockaddr*)&serv_addr, len);
    if (m_len == -1) {
        LENNY_LOG_WARNING("Something went wrong when sending message `%s` to server", message.c_str())
        return;
    }

    //Receive response
    if (waitForResponse) {
        char r_buff[Utils::UDP_BUFFER_SIZE];
        const int r_len = recvfrom(socket_fd, r_buff, Utils::UDP_BUFFER_SIZE, 0, (struct sockaddr*)&serv_addr, &len);
        if (r_len == -1) {
            LENNY_LOG_WARNING("Something went wrong when receiving response from server")
            return;
        }
        response = std::string(r_buff).substr(0, r_len);

        //Print response
        if (Utils::UDP_PRINT_MESSAGES)
            LENNY_LOG_PRINT(tools::Logger::MAGENTA, "CLIENT (response from server): `%s`\n", response.value().c_str())
    }
}

}  // namespace lenny::communication