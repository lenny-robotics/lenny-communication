#include <lenny/communication/TCPClient.h>
#include <lenny/communication/Utils.h>

#include <iostream>

int main() {
    lenny::communication::Utils::TCP_PRINT_MESSAGES = true;
    lenny::communication::TCPClient client("127.0.0.1", 8080);
    client.connect();
    while (client.isConnected()) {
        std::cout << "Send message to server (`exit` to terminate):" << std::endl;
        std::string message;
        std::getline(std::cin, message);
        if (message == "exit")
            break;
        std::optional<std::string> response;
        client.writeToServer(response, message);
        if (response.has_value()) {
            std::cout << "Received response from server: " << response.value() << std::endl;
        } else {
            std::cout << "Received no response from server..." << std::endl;
        }
    }
    client.disconnect();
    return 0;
}