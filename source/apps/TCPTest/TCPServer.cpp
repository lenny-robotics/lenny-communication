#include <lenny/communication/TCPServer.h>
#include <lenny/communication/Utils.h>

int main() {
    lenny::communication::Utils::TCP_PRINT_MESSAGES = true;
    const bool sendResponse = true;

    lenny::communication::TCPServer server(8080);
    server.f_messageFromClient = [&](std::optional<std::string>& response, const std::string& message) -> void {
        if (sendResponse)
            response = message;
        else
            response = std::nullopt;
    };
    server.open();
    server.loop();
    server.close();
    return 0;
}