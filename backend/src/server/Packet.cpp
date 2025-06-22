#include "Packet.h"

#include "log/Log.h"
#include "utils/endian.h"

using namespace Endian;

void PacketBuffer::append(const char* data, const size_t length) {
    if (isReady()) reset();

    buffer.insert(buffer.end(), data, data + length);

    if (state == State::READING_HEADER) {
        if (buffer.size() >= sizeof(uint32_t)) {
            memcpy(&exp_size, buffer.data(), sizeof(uint32_t));
            exp_size = ntohl(exp_size);

            // 헤더만큼 버퍼에서 제거
            buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t));

            state = State::READING_PAYLOAD;
        }
    }

    if (state == State::READING_PAYLOAD) {
        // 페이로드를 완성할 만큼 데이터가 모였는지 확인
        if (buffer.size() >= exp_size) {
            state = State::READY;
        }
    }
}

Packet PacketBuffer::toPacket(const sockfd fd) const {
    const auto string_packet = std::string(buffer.begin(), buffer.end());
    json packet = json::parse(string_packet);
    if (!packet.contains("event")) {
        packet["event"] = "";
    }
    if (!packet.contains("data")) {
        packet["data"] = "";
    }

    return {packet["event"].get<std::string>(), packet["data"].get<std::string>(), fd};
}

size_t PacketBuffer::remainingSize() const {
    if (state == State::READING_HEADER) {
        return sizeof(uint32_t) - buffer.size();
    }

    return exp_size - size();
}

void PacketBuffer::reset() {
    state = State::READING_HEADER;
    buffer.clear();
    exp_size = 0;
}
