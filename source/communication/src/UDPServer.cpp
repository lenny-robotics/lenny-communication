#include <lenny/communication/UDPServer.h>
#include <lenny/communication/Utils.h>
#include <lenny/tools/Logger.h>

#if WIN32
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace lenny::communication {

UDPServer::UDPServer(const int &port) : port(port) {
    //Set f_messageFromClient as test function
    f_messageFromClient = [](std::optional<std::string> &response, const std::string &message, const std::string &ip, const std::uint16_t &port) -> void {
        response = message;  //Echo received message
    };
}

UDPServer::~UDPServer() {
    if (isOpen())
        close();
}

bool UDPServer::open() {
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
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LENNY_LOG_WARNING("Socket creation failed")
        return false;
    }

    //Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LENNY_LOG_WARNING("Socket binding failed")
        return false;
    }

    //Wrap up
    opened = true;
    LENNY_LOG_INFO("UDPServer successfully opened on port `%d`", port)
    return true;
}

void UDPServer::close() {
    if (!isOpen())
        return;

    opened = false;
#if WIN32
    shutdown(server_fd, SD_BOTH);
    closesocket(server_fd);
    WSACleanup();
#else
    shutdown(server_fd, SHUT_RDWR);
    ::close(server_fd);
#endif
    LENNY_LOG_INFO("UDPServer successfully disconnected on port `%d`", port)
}

bool UDPServer::isOpen() const {
    return opened;
}

void UDPServer::loop() {
    if (!isOpen()) {
        LENNY_LOG_WARNING("Open server before running loop")
        return;
    }

    auto ipToString = [](sockaddr_in addr) -> std::string {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
        return std::string(ip);
    };

    struct sockaddr_in clientAddress;
    socklen_t len = sizeof(clientAddress);
    while (opened) {
        //Receive message from client
        char m_buff[Utils::UDP_BUFFER_SIZE];
        const int m_len = recvfrom(server_fd, m_buff, sizeof(m_buff), 0, (struct sockaddr *)&clientAddress, &len);
        if (m_len > 0) {
            //Gather information
            const std::string message(std::string(m_buff).substr(0, m_len));
            const std::string clientIP = ipToString(clientAddress);
            const std::uint16_t clientPort = ntohs(clientAddress.sin_port);
            if (Utils::UDP_PRINT_MESSAGES)
                LENNY_LOG_PRINT(tools::Logger::MAGENTA, "SERVER (received from client with ip `%s` and port `%d`): `%s`\n", clientIP.c_str(), clientPort,
                                message.c_str())

            //Callback
            std::optional<std::string> response = std::nullopt;
            if (f_messageFromClient)
                f_messageFromClient(response, message, clientIP, clientPort);

            //Send response to client
            if (response.has_value()) {
                const int r_val = sendto(server_fd, response.value().c_str(), response.value().size(), 0, (const struct sockaddr *)&clientAddress, len);
                if (r_val == -1)
                    LENNY_LOG_WARNING("Something went wrong when sending response to client with IP `%s` and port `%d`", clientIP.c_str(), clientPort)

                if (Utils::UDP_PRINT_MESSAGES)
                    LENNY_LOG_PRINT(tools::Logger::MAGENTA, "SERVER (response to client with ip `%s` and port `%d`): `%s`\n", clientIP.c_str(), clientPort,
                                    response.value().c_str())
            }
        }
    }
}

}  // namespace lenny::communication