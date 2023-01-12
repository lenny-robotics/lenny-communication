#pragma once

#ifdef LENNY_COMMUNICATION_USE_WEBRTC

#include <functional>
#include <memory>
#include <rtc/rtc.hpp>
#include <string>
#include <unordered_map>

namespace lenny::communication {

class WebRTCClient {
public:
    WebRTCClient(const std::string& localID) : localID(localID) {}
    ~WebRTCClient();

    void initializeRTCLogger();
    void connectToWebServer(const std::string& ip, const std::string& port);

    std::shared_ptr<rtc::PeerConnection> createPeerConnection(const std::string& peerID);
    std::shared_ptr<rtc::DataChannel> createDataChannel(const std::string& peerID, const std::string& label);

public:
    typedef std::function<void(rtc::message_variant data)> OnDataChannelMessage;
    const std::string localID;
    std::shared_ptr<rtc::WebSocket> webSocket;
    std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> peerConnectionMap;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> dataChannelMap;
    std::unordered_map<std::string, OnDataChannelMessage> onDataChannelMessageMap;
};

}  // namespace lenny::communication

#endif