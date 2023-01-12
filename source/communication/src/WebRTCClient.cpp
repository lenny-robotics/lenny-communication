#include <lenny/communication/WebRTCClient.h>

#ifdef LENNY_COMMUNICATION_USE_WEBRTC

#include <lenny/tools/Json.h>
#include <lenny/tools/Logger.h>

#include <future>

namespace lenny::communication {

template <class T>
std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) {
    return ptr;
}

WebRTCClient::~WebRTCClient() {
    dataChannelMap.clear();
    peerConnectionMap.clear();
}

void WebRTCClient::initializeRTCLogger() {
    rtc::InitLogger(rtc::LogLevel::Info);
}

void WebRTCClient::connectToWebServer(const std::string& ip, const std::string& port) {
    //Setup web socket
    std::promise<void> wsPromise;
    std::future<void> wsFuture = wsPromise.get_future();

    webSocket = std::make_shared<rtc::WebSocket>();

    webSocket->onOpen([&wsPromise]() {
        LENNY_LOG_INFO("WebSocket connected, signaling ready")
        wsPromise.set_value();
    });

    webSocket->onError([&wsPromise](std::string s) {
        LENNY_LOG_WARNING("WebSocket error")
        wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    webSocket->onClosed([]() { LENNY_LOG_INFO("WebSocket closed") });

    webSocket->onMessage([&](auto data) {
        //Data holds either std::string or rtc::binary
        if (!std::holds_alternative<std::string>(data))
            return;

        //Parse json message
        const json message = json::parse(std::get<std::string>(data));

        //Get id
        auto it = message.find("id");
        if (it == message.end())
            return;
        const std::string id = it->get<std::string>();

        //Get type
        it = message.find("type");
        if (it == message.end())
            return;
        const std::string type = it->get<std::string>();

        //Check if peer connection exists, if not, create it
        std::shared_ptr<rtc::PeerConnection> pc;
        if (auto jt = peerConnectionMap.find(id); jt != peerConnectionMap.end()) {
            pc = jt->second;
        } else if (type == "offer") {
            LENNY_LOG_INFO("Answering to '%s'", id.c_str())
            pc = createPeerConnection(id);
        } else {
            return;
        }

        // Setup depending on type
        if (type == "offer" || type == "answer") {
            auto sdp = message["description"].get<std::string>();
            pc->setRemoteDescription(rtc::Description(sdp, type));
        } else if (type == "candidate") {
            auto sdp = message["candidate"].get<std::string>();
            auto mid = message["mid"].get<std::string>();
            pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
        }
    });

    //Open web socket
    const std::string url = "ws://" + ip + ":" + port + "/" + localID;
    LENNY_LOG_DEBUG("Opening web socket with url '%s'", url.c_str());
    webSocket->open(url);

    LENNY_LOG_DEBUG("Waiting for signaling to be connected...");
    wsFuture.get();
}

std::shared_ptr<rtc::PeerConnection> WebRTCClient::createPeerConnection(const std::string& peerID) {
    rtc::Configuration config;
    std::shared_ptr<rtc::PeerConnection> pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onStateChange([peerID](rtc::PeerConnection::State state) {
        std::stringstream ss;
        ss << state;
        LENNY_LOG_DEBUG("Connection State (id '%s'): %s", peerID.c_str(), ss.str().c_str())
    });

    pc->onGatheringStateChange([peerID](rtc::PeerConnection::GatheringState state) {
        std::stringstream ss;
        ss << state;
        LENNY_LOG_DEBUG("Gathering State (id '%s'): %s", peerID.c_str(), ss.str().c_str())
    });

    pc->onLocalDescription([wws = make_weak_ptr(webSocket), peerID](rtc::Description description) {
        const json message = {{"id", peerID}, {"type", description.typeString()}, {"description", std::string(description)}};
        if (auto ws = wws.lock())
            ws->send(message.dump());
    });

    pc->onLocalCandidate([wws = make_weak_ptr(webSocket), peerID](rtc::Candidate candidate) {
        const json message = {{"id", peerID}, {"type", "candidate"}, {"candidate", std::string(candidate)}, {"mid", candidate.mid()}};
        if (auto ws = wws.lock())
            ws->send(message.dump());
    });

    pc->onDataChannel([&, peerID](std::shared_ptr<rtc::DataChannel> dc) {
        LENNY_LOG_INFO("[ONDATACHANNEL] DataChannel from '%s' received with label '%s'", peerID.c_str(), dc->label().c_str())

        dc->onOpen([&, wdc = make_weak_ptr(dc)]() {
            if (auto dc = wdc.lock())
                dc->send("[ONDATACHANNEL]: Hello from " + localID);
        });

        dc->onClosed([peerID]() { LENNY_LOG_INFO("[ONDATACHANNEL]: DataChannel from '%s' closed", peerID.c_str()) });

        dc->onMessage([&, peerID](auto data) {
            if (onDataChannelMessageMap.find(peerID) != onDataChannelMessageMap.end() && onDataChannelMessageMap.at(peerID)) {
                onDataChannelMessageMap.at(peerID)(data);
            } else {
                //Data holds either std::string or rtc::binary
                if (std::holds_alternative<std::string>(data))
                    LENNY_LOG_INFO("[ONDATACHANNEL] Message from '%s' received: %s", peerID.c_str(), std::get<std::string>(data).c_str())
                else
                    LENNY_LOG_INFO("[ONDATACHANNEL] Binary message from '%s' received (Size: %d)", peerID.c_str(), std::get<rtc::binary>(data).size())
            }
        });

        dataChannelMap.emplace(peerID, dc);
    });

    peerConnectionMap.emplace(peerID, pc);
    return pc;
}

std::shared_ptr<rtc::DataChannel> WebRTCClient::createDataChannel(const std::string& peerID, const std::string& label) {
    LENNY_LOG_DEBUG("Creating DataChannel for id '%s' with label '%s'", peerID.c_str(), label.c_str())
    std::shared_ptr<rtc::PeerConnection> pc;
    if (peerConnectionMap.find(peerID) == peerConnectionMap.end()) {
        LENNY_LOG_DEBUG("Peer connection with id '%s' not found, creating it first...", peerID.c_str())
        pc = createPeerConnection(peerID);
    } else {
        pc = peerConnectionMap.at(peerID);
    }

    auto dc = pc->createDataChannel(label);
    dc->onOpen([&, peerID, wdc = make_weak_ptr(dc)]() {
        LENNY_LOG_INFO("DataChannel from id '%s' open", peerID.c_str())
        if (auto dc = wdc.lock())
            dc->send("Hello from " + localID);
    });

    dc->onClosed([peerID]() { LENNY_LOG_INFO("DataChannel from '%s' closed", peerID.c_str()) });

    dc->onMessage([peerID, wdc = make_weak_ptr(dc)](auto data) {
        //Data holds either std::string or rtc::binary
        if (std::holds_alternative<std::string>(data))
            LENNY_LOG_INFO("Message from '%s' received: %s", peerID.c_str(), std::get<std::string>(data).c_str())
        else
            LENNY_LOG_INFO("Binary message from '%s' received (Size: %d)", peerID.c_str(), std::get<rtc::binary>(data).size())
    });

    dataChannelMap.emplace(peerID, dc);
    return dc;
}

}  // namespace lenny::communication

#endif