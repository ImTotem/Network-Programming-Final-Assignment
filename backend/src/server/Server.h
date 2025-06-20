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


class Server : public EventEmitter {
    const std::unique_ptr<Socket> socket;
    const std::unique_ptr<IOMultiplexer> io = std::make_unique<IOMultiplexer>();
    std::vector<char> recv_buffer;
public:
    explicit Server(unsigned short port): socket(std::make_unique<Socket>(port)), recv_buffer(4096) {}
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
        // 매우 단순한 JSON 직렬화 (실제 환경에서는 JSON 라이브러리 권장)
        return R"({"event":")" + event + R"(","data":")" + base64data + "\"}";
    }
    static std::pair<std::string, std::string> parsePacket(const std::string& raw) {
        // 매우 단순한 파싱: {"event":"...","data":"..."}
        auto epos = raw.find(R"("event":")");
        auto dpos = raw.find(R"("data":")");
        if (epos == std::string::npos || dpos == std::string::npos) return {"", ""};
        epos += 9;
        const auto eend = raw.find('"', epos);
        const auto dend = raw.find('"', dpos + 8);
        if (eend == std::string::npos || dend == std::string::npos) return {"", ""};
        std::string event = raw.substr(epos, eend - epos);
        std::string data = raw.substr(dpos + 8, dend - (dpos + 8));
        return {event, data};
    }

private:
    void handleNewConnection();
    void handleStdinInput();
};



#endif //SERVER_H
