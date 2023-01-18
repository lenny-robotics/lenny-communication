#include <lenny/communication/UDPServer.h>
#include <lenny/communication/Utils.h>

int main() {
    lenny::communication::Utils::UDP_PRINT_MESSAGES = true;
    const bool sendResponse = true;

    lenny::communication::UDPServer server(8080);
    server.f_messageFromClient = [&](std::optional<std::string> &response, const std::string &message, const std::string &ip,
                                     const std::uint16_t &port) -> void {
        if (sendResponse)
            response = message;  //Echo received message
        else
            response = std::nullopt;
    };
    server.open();
    server.loop();
    server.close();
    return 0;
}