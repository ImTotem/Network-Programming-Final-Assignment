//
// Created by 박성빈 on 25. 6. 19.
//

#include "WebSocket.h"
#include <cstring>
#include "log/Log.h"
#include "utils/sha1.h"
#include "utils/base64.h"
#include "utils/Endian.h"
#include "room/RoomServer.h"
#include "room/client/Client.h"

namespace WebSocketMiddleware {

    void handleHandshake(Context& ctx) {
        if (ctx.client->isWebSocket) return;

        std::string request(ctx.payload.begin(), ctx.payload.end());
        if (request.rfind("GET /", 0) != 0 || request.find("Upgrade: websocket") == std::string::npos) {
            return;
        }

        std::string key;
        auto key_pos = request.find("Sec-WebSocket-Key: ");
        if (key_pos != std::string::npos) {
            auto key_start = key_pos + 19;
            auto key_end = request.find("\r\n", key_start);
            if (key_end != std::string::npos) key = request.substr(key_start, key_end - key_start);
        }

        if (key.empty()) return;

        const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        SHA1 checksum;
        checksum.update(key + magic);
        const std::string accept_key_str = base64::to_base64(checksum.final());

        std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
                               "Upgrade: websocket\r\n"
                               "Connection: Upgrade\r\n"
                               "Sec-WebSocket-Accept: " + accept_key_str + "\r\n\r\n";

        ctx.server.write_raw(ctx.client->fd, response);
        ctx.client->isWebSocket = true;
        Log::info("WebSocket handshake successful for fd: {}", ctx.client->fd);
        ctx.stopProcessing = true;
    }

    void handleFrames(Context& ctx) {
        if (!ctx.client->isWebSocket) return;

        ctx.client->websocket_buffer.insert(ctx.client->websocket_buffer.end(), ctx.payload.begin(), ctx.payload.end());
        std::vector<char> application_payload;
        bool processed_frame = false;

        while (true) {
            auto& buffer = ctx.client->websocket_buffer;
            if (buffer.size() < 2) break;

            uint8_t opcode = buffer[0] & 0x0F;
            bool is_masked = (buffer[1] & 0x80) != 0;
            uint64_t payload_len = buffer[1] & 0x7F;
            size_t offset = 2;

            if (payload_len == 126) {
                if (buffer.size() < 4) break;
                uint16_t len;
                memcpy(&len, &buffer[2], 2);
                payload_len = Endian::networkToHost16(len);
                offset = 4;
            } else if (payload_len == 127) {
                if (buffer.size() < 10) break;
                uint64_t len;
                memcpy(&len, &buffer[2], 8);
                payload_len = Endian::networkToHost64(len);
                offset = 10;
            }

            if (!is_masked) { ctx.server.close(ctx.client->fd); return; }

            size_t frame_size = offset + 4 + payload_len;
            if (buffer.size() < frame_size) break;

            char masking_key[4];
            memcpy(masking_key, buffer.data() + offset, 4);
            offset += 4;

            std::vector<char> unmasked_payload(payload_len);
            for (uint64_t i = 0; i < payload_len; ++i) {
                unmasked_payload[i] = buffer[offset + i] ^ masking_key[i % 4];
            }

            if (opcode == 0x2) { // Binary Frame
                application_payload.insert(application_payload.end(), unmasked_payload.begin(), unmasked_payload.end());
                processed_frame = true;
            } else if (opcode == 0x8) { // Close
                ctx.server.close(ctx.client->fd);
                return;
            } else if (opcode == 0x9) { // Ping
                auto pong_frame = createFrame(unmasked_payload, 0xA); // 0xA = Pong
                ctx.server.write_raw(ctx.client->fd, std::string(pong_frame.begin(), pong_frame.end()));
            }

            buffer.erase(buffer.begin(), buffer.begin() + frame_size);
        }

        if (processed_frame) {
            ctx.payload = application_payload;
        } else {
            ctx.stopProcessing = true;
        }
    }

    std::vector<char> createFrame(const std::vector<char>& payload, uint8_t opcode) {
        std::vector<char> frame;
        frame.push_back(0x80 | opcode);

        if (payload.size() <= 125) {
            frame.push_back(static_cast<char>(payload.size()));
        } else if (payload.size() <= 65535) {
            frame.push_back(126);
            uint16_t len = Endian::hostToNetwork16(payload.size());
            frame.insert(frame.end(), reinterpret_cast<char*>(&len), reinterpret_cast<char*>(&len) + 2);
        } else {
            frame.push_back(127);
            uint64_t len = Endian::hostToNetwork64(payload.size());
            frame.insert(frame.end(), reinterpret_cast<char*>(&len), reinterpret_cast<char*>(&len) + 8);
        }

        frame.insert(frame.end(), payload.begin(), payload.end());
        return frame;
    }
}
