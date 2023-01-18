#include <lenny/communication/UDPClient.h>
#include <lenny/communication/Utils.h>

#include <iostream>

int main() {
    lenny::communication::Utils::UDP_PRINT_MESSAGES = true;
    const bool waitForResponse = true;

    lenny::communication::UDPClient client("127.0.0.1", 8080);
    client.open();
    while (client.isOpen()) {
        std::cout << "Send message to server (`exit` to terminate):" << std::endl;
        std::string message;
        std::getline(std::cin, message);
        if (message == "exit")
            break;
        std::optional<std::string> response;
        client.writeToServer(response, message, waitForResponse);
        if (response.has_value()) {
            std::cout << "Received response from server: " << response.value() << std::endl;
        } else if(waitForResponse) {
            std::cout << "Received no response from server..." << std::endl;
        }
    }
    client.close();
    return 0;
}