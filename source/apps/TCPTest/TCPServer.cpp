#include <lenny/communication/TCPServer.h>
#include <lenny/communication/Utils.h>

int main() {
    lenny::communication::Utils::TCP_PRINT_MESSAGES = true;
    lenny::communication::TCPServer server(8080);
    server.open();
    server.loop();
    server.close();
    return 0;
}