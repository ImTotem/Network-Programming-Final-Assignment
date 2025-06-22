//
// Created by 박성빈 on 25. 6. 20.
//

#ifndef PACKET_H
#define PACKET_H
#include <string>
#include <vector>
#include "utils/json.hpp"

#include "socket/SocketType.h"

using json = nlohmann::json;

struct Packet {
    std::string event;
    std::string data; // base64 string
    sockfd fd;
    Packet(std::string event, std::string data, const sockfd fd) : event(std::move(event)), data(std::move(data)), fd(fd) {}
};

class PacketBuffer {
public:
    enum class State {
        READING_HEADER,
        READING_PAYLOAD,
        READY
    };
private:
    // 버퍼 상태
    State state = State::READING_HEADER;
    // 앞으로 받아야 할 본문의 전체 크기
    uint32_t exp_size = 0;
public:
    // 헤더/본문 데이터를 임시 저장할 버퍼
    std::vector<char> buffer{};

    void append(const char* data, size_t length);

    [[nodiscard]] Packet toPacket(sockfd fd) const;

    [[nodiscard]] State getState() const { return state; }
    [[nodiscard]] bool isReady() const { return state == State::READY; }
    [[nodiscard]] size_t size() const {
        return buffer.size();
    }
    [[nodiscard]] size_t remainingSize() const;

    void reset();
};

#endif //PACKET_H
