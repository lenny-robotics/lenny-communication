#include <lenny/communication/WebRTCClient.h>

#include <random>

std::string randomId(size_t length) {
    using std::chrono::high_resolution_clock;
    static thread_local std::mt19937 rng(static_cast<unsigned int>(high_resolution_clock::now().time_since_epoch().count()));
    static const std::string characters("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::string id(length, '0');
    std::uniform_int_distribution<int> uniform(0, int(characters.size() - 1));
    std::generate(id.begin(), id.end(), [&]() { return characters.at(uniform(rng)); });
    return id;
}

int main() {
    lenny::communication::WebRTCClient client(randomId(4));
    client.connectToWebServer("127.0.0.1", "8000");

    std::string peerID = "";
    while (true) {
        if (client.dataChannelMap.find(peerID) == client.dataChannelMap.end()) {
            std::cout << "Enter a remote ID to send an offer:" << std::endl;
            std::cin >> peerID;
            std::cin.ignore();

            if (peerID.empty())
                break;

            if (peerID == client.localID) {
                std::cout << "Invalid remote ID (This is the local ID)" << std::endl;
                continue;
            }

            client.createDataChannel(peerID, randomId(4));
        } else {
            std::string message;
            std::cout << "Enter message:" << std::endl;
            std::getline(std::cin, message);
            auto dc = client.dataChannelMap.at(peerID);
            dc->send(message);
        }
    }
}