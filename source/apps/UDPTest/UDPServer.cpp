#include <lenny/communication/UDPServer.h>
#include <lenny/communication/Utils.h>

int main() {
    lenny::communication::Utils::UDP_PRINT_MESSAGES = true;
    lenny::communication::UDPServer server(8080);
    server.open();
    server.loop();
    server.close();
    return 0;
}