//
// Created by 박성빈 on 25. 6. 12.
//

#ifndef SERVER_H
#define SERVER_H
#include <memory>

#include "Packet.h"
#include "event/EventEmitter.h"
#include "socket/Socket.h"
#include "socket/IOMultiplexer/IOMultiplexer.h"

#include "utils/json.hpp"

#include "log/Log.h"
using json = nlohmann::json;

class Server : public EventEmitter {
    const std::unique_ptr<Socket> socket;
    const std::unique_ptr<IOMultiplexer> io = std::make_unique<IOMultiplexer>();
    // 클라이언트별로 독립적인 버퍼를 관리
    std::map<sockfd, PacketBuffer> client_buffers;

public:
    explicit Server(unsigned short port): socket(std::make_unique<Socket>(port)) {}
    ~Server() override = default;

    [[noreturn]] void start();
    void stop();
    void close(sockfd fd);

    void send(const Packet& packet);
    void recv(sockfd fd);

    [[nodiscard]] virtual std::vector<sockfd> getAllFd() const {
        return {};
    }

    // --- 패킷 파서/빌더 ---
    static std::string serializePacket(const std::string& event, const std::string& base64data) {
        json packet;
        packet["event"] = event;
        packet["data"] = base64data;
        return packet.dump();
    }
    static std::pair<std::string, std::string> parsePacket(const std::string& raw) {
        json packet = json::parse(raw);
        if (!packet.contains("event")) {
            packet["event"] = "";
        }
        if (!packet.contains("data")) {
            packet["data"] = "";
        }
        return {packet["event"].get<std::string>(), packet["data"].get<std::string>()};
    }

private:
    void handleNewConnection();
    void handleStdinInput();
};



#endif //SERVER_H
