#include <lenny/communication/TCPServer.h>
#include <lenny/communication/Utils.h>
#include <lenny/tools/Logger.h>

#if WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace lenny::communication {

TCPServer::TCPServer(const int &port) : port(port) {
    //Set f_messageFromClient as test function
    f_messageFromClient = [](std::optional<std::string> &response, const std::string &message) -> void {
        response = message;  //Echo message
    };
}

TCPServer::~TCPServer() {
    if (isOpen())
        close();
}

inline struct sockaddr_in getAddress(const int port) {
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    return address;
}

bool TCPServer::open() {
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
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        LENNY_LOG_WARNING("Socket creation failed")
        return false;
    }

    //Bind socket
    struct sockaddr_in address = getAddress(port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LENNY_LOG_WARNING("Socket binding failed")
        return false;
    }

    //Prepare to accept connections
    if (listen(server_fd, 3) < 0) {
        LENNY_LOG_WARNING("Preparation to accept connections failed")
        return false;
    }

    //Wrap up
    opened = true;
    LENNY_LOG_INFO("TCPServer successfully opened on port `%d`", port)
    return true;
}

void TCPServer::close() {
    if (!isOpen())
        return;

    opened = false;
#if WIN32
    shutdown(socket_fd, SD_BOTH);
    closesocket(socket_fd);
    shutdown(server_fd, SD_BOTH);
    closesocket(server_fd);
    WSACleanup();
#else
    shutdown(socket_fd, SHUT_RDWR);
    ::close(socket_fd);
    shutdown(server_fd, SHUT_RDWR);
    ::close(server_fd);
#endif
    LENNY_LOG_INFO("TCPServer successfully disconnected on port `%d`", port)
}

bool TCPServer::isOpen() const {
    return opened;
}

bool TCPServer::isClientConnected() const {
    return clientConnected;
}

void TCPServer::loop() {
    if (!isOpen()) {
        LENNY_LOG_WARNING("Open server before running loop")
        return;
    }

    struct sockaddr_in address = getAddress(port);
#if WIN32
    int addrlen = sizeof(address);
#else
    socklen_t addrlen = sizeof(address);
#endif
    while (opened) {
        //Establish client connection
        if (!isClientConnected()) {
            LENNY_LOG_DEBUG("Waiting for client to connect...")
            if ((socket_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) >= 0) {
                LENNY_LOG_DEBUG("Client connected")
                clientConnected = true;
            }
        } else {  //Exchange messages
            //Wait for message from client
            char m_buff[Utils::TCP_BUFFER_SIZE];
            const int m_len = recv(socket_fd, m_buff, sizeof(m_buff), 0);
            if (m_len > 0) {
                const std::string message(std::string(m_buff).substr(0, m_len));
                if (Utils::TCP_PRINT_MESSAGES)
                    LENNY_LOG_PRINT(tools::Logger::MAGENTA, "SERVER (received from client): `%s`\n", message.c_str())

                //Parse message and setup response
                std::optional<std::string> response = std::nullopt;
                if (message == Utils::TCP_CLIENT_CLOSE_MESSAGE) {
                    LENNY_LOG_DEBUG("Client is closing...")
                    clientConnected = false;
                } else if (f_messageFromClient) {  //If not handle message in a customized way
                    f_messageFromClient(response, message);
                }

                //If there is a response, send it back to the client
                if (response.has_value()) {
                    //Check message length
                    if (response.value().size() > Utils::TCP_BUFFER_SIZE) {
                        LENNY_LOG_WARNING("Message that should be sent to client is too long (%d VS %d): `%s`", response.value().size(), Utils::TCP_BUFFER_SIZE,
                                          response.value().c_str())
                        continue;
                    }

                    //Send response
                    const int r_len = send(socket_fd, response.value().c_str(), response.value().size(), 0);
                    if (r_len == -1)
                        LENNY_LOG_WARNING("Something went wrong when sending response `%s` to client", response.value().c_str())

                    //Print
                    if (Utils::TCP_PRINT_MESSAGES)
                        LENNY_LOG_PRINT(tools::Logger::MAGENTA, "SERVER (response to client): `%s`\n", response.value().c_str())
                }
            }
        }
    }
}

}  // namespace lenny::communication