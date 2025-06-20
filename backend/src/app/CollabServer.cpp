//
// Created by 박성빈 on 25. 6. 11.
//

#include "CollabServer.h"
#include <string>
#include "env/dotenv.h"
#include "log/Log.h"
#include "room/RoomServer.h"
#include "room/client/Client.h"
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>

// --- 유틸 함수 구현 ---
static std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

static std::string join(const std::vector<std::string>& v, const std::string& delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i != 0) oss << delim;
        oss << v[i];
    }
    return oss.str();
}

static std::string extractSocketId(const std::string& json) {
    // 매우 단순한 string 파싱: "socketId":"..." 패턴에서 ...만 추출
    auto pos = json.find("\"socketId\":\"");
    if (pos == std::string::npos) return "";
    pos += 13; // "socketId":" 길이
    auto end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

CollabServer::CollabServer() {
    // 1. 환경변수 로드
    const std::string prod_env = ".env.production";
    const std::string dev_env = ".env.development";
    env_load(getenv("DEBUG") ? dev_env.c_str() : prod_env.c_str(), true);

    // 2. 포트 설정
    unsigned short port;
    if (getenv("PORT")) {
        port = static_cast<unsigned short>(strtoul(getenv("PORT"), nullptr, 0));
    } else {
        port = 3002;
    }

    // 3. RoomServer 엔진 생성
    io = std::make_unique<RoomServer>(port);

    // 4. 이벤트 핸들러 설정
    setupEventHandlers();
}

CollabServer::~CollabServer() = default;

void CollabServer::run() {
    Log::info("CollabServer is running...");
    io->start(); // 서버 엔진의 메인 루프 시작
}

void CollabServer::setupEventHandlers() {
    io->on("connection", [this](const Packet& packet) {
        auto client = io->getClient(packet.fd);
        if (!client) return;
        Log::info("Connection established! Client ID: {}", client->id);
        io->emitToRoom(client->id, Packet("init-room", "", client->fd));

        client->on("join-room", [this, client](const Packet& packet) {
            const std::string& roomID = packet.data;
            Log::debug("{} has joined {}", client->id, roomID);
            client->join(roomID);
            const auto sockets = io->socketsIn(roomID);
            if (sockets.size() <= 1) {
                io->emitToRoom(client->id, Packet("first-in-room", "", client->fd));
            } else {
                Log::debug("{} emitting new-user to room {}", client->id, roomID);
                io->broadcastToRoom(roomID, Packet("new-user", client->id, client->fd));
            }
            std::vector<std::string> socket_ids;
            socket_ids.reserve(sockets.size());
            for (const auto& s : sockets) socket_ids.push_back(s->id);
            io->emitToRoom(roomID, Packet("room-user-change", join(socket_ids, ","), client->fd));
        });

        client->on("server-broadcast", [this, client](const Packet& packet) {
            auto parts = split(packet.data, '|');
            if (parts.size() != 3) return;
            const auto& roomID = parts[0];
            const auto& encryptedData = parts[1];
            const auto& iv = parts[2];
            Log::debug("{} sends update to {}", client->id, roomID);
            io->broadcastToRoom(roomID, Packet("client-broadcast", encryptedData + "|" + iv, client->fd));
        });

        client->on("server-volatile-broadcast", [this, client](const Packet& packet) {
            auto parts = split(packet.data, '|');
            if (parts.size() != 3) return;
            const auto& roomID = parts[0];
            const auto& encryptedData = parts[1];
            const auto& iv = parts[2];
            Log::debug("{} sends volatile update to {}", client->id, roomID);
            io->broadcastToRoom(roomID, Packet("client-broadcast", encryptedData + "|" + iv, client->fd));
        });

        client->on("user-follow", [this, client](const Packet& packet) {
            const std::string& json_string = packet.data;
            std::string roomID = "follow@" + extractSocketId(json_string);
            if (json_string.find("FOLLOW") != std::string::npos) {
                client->join(roomID);
            } else {
                client->leave(roomID);
            }
            const auto sockets = io->socketsIn(roomID);
            std::vector<std::string> followedBy_ids;
            for (const auto& s : sockets) followedBy_ids.push_back(s->id);
            io->emitToRoom(extractSocketId(json_string), Packet("user-follow-room-change", join(followedBy_ids, ","), client->fd));
        });

        client->on("disconnecting", [this, client](const Packet&) {
            Log::debug("{} has disconnected", client->id);
            for (const auto& roomID : client->rooms) {
                const auto otherClients = io->socketsIn(roomID);
                const bool isFollowRoom = roomID.rfind("follow@", 0) == 0;
                if (!isFollowRoom && otherClients.size() > 1) {
                    std::vector<std::string> socket_ids;
                    for (const auto& c : otherClients) {
                        if (c->id != client->id) socket_ids.push_back(c->id);
                    }
                    if (!socket_ids.empty()) {
                        io->broadcastToRoom(roomID, Packet("room-user-change", join(socket_ids, ","), client->fd));
                    }
                }
                if (isFollowRoom && otherClients.size() == 1 && otherClients[0]->id == client->id) {
                    const auto socketId = roomID.substr(std::string("follow@").length());
                    io->emitToRoom(socketId, Packet("broadcast-unfollow", "", client->fd));
                }
            }
        });
    });
}

