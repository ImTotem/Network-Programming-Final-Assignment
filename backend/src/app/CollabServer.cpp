//
// Created by 박성빈 on 25. 6. 11.
//

#include "CollabServer.h"
#include <string>
#include "env/dotenv.h"
// #include "log/Log.h"
#include "room/RoomServer.h"
#include "room/client/Client.h"
#include <vector>

#include "utils/base64.h"
#include "utils/json.hpp"

using json = nlohmann::json;

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

        io->emitToRoom(client->id, Packet("init-room", base64::encode(json::to_cbor(client->id)), client->fd));

        client->on("join-room", [this, client](const Packet& pack) {
            const std::string& roomID = json::from_cbor(base64::decode(pack.data)).get<std::string>();
            Log::debug("{} has joined {}", client->id, roomID);
            client->join(roomID);
            const auto sockets = io->socketsIn(roomID);
            Log::debug("{}", sockets.size());
            if (sockets.size() <= 1) {
                io->emitToRoom(client->id, Packet("first-in-room", base64::encode(json::to_cbor("")), client->fd));
            } else {
                Log::debug("{} emitting new-user to room {}", client->id, roomID);
                io->broadcastToRoom(roomID, Packet("new-user", base64::encode(json::to_cbor(client->id)), client->fd));
            }
            std::vector<std::string> socket_ids;
            socket_ids.reserve(sockets.size());
            for (const auto& s : sockets) socket_ids.push_back(s->id);
            io->emitToRoom(roomID, Packet("room-user-change", base64::encode(json::to_cbor(socket_ids)), client->fd));
        });

        client->on("server-broadcast", [this, client](const Packet& packet) {
            const std::string roomID = json::from_cbor(base64::decode(packet.data))["roomId"].get<std::string>();
            Log::debug("{} sends update to {}", client->id, roomID);
            io->broadcastToRoom(roomID, Packet("client-broadcast", packet.data, client->fd));
        });

        client->on("server-volatile-broadcast", [this, client](const Packet& packet) {
            const std::string roomID = json::from_cbor(base64::decode(packet.data))["roomId"].get<std::string>();
            Log::debug("{} sends volatile update to {}", client->id, roomID);
            io->broadcastToRoom(roomID, Packet("client-broadcast", packet.data, client->fd));
        });

        client->on("user-follow", [this, client](const Packet& packet) {
            const auto data = json::from_cbor(base64::decode(packet.data));
            const std::string roomID = "follow@" + data["userToFollow"]["socketId"].get<std::string>();
            if (data["action"].get<std::string>() == "FOLLOW") {
                client->join(roomID);
            } else if (data["action"].get<std::string>() == "UNFOLLOW") {
                client->leave(roomID);
            }
            const auto sockets = io->socketsIn(roomID);
            std::vector<std::string> followedBy_ids;
            for (const auto& s : sockets) followedBy_ids.push_back(s->id);
            io->emitToRoom(
                data["userToFollow"]["socketId"].get<std::string>(),
                Packet("user-follow-room-change", base64::encode(json::to_cbor(followedBy_ids)), client->fd)
            );
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
                        io->broadcastToRoom(roomID, Packet("room-user-change", base64::encode(json::to_cbor(socket_ids)), client->fd));
                    }
                }
                if (isFollowRoom && otherClients.size() == 1 && otherClients[0]->id == client->id) {
                    const auto socketId = roomID.substr(std::string("follow@").length());
                    io->emitToRoom(socketId, Packet("broadcast-unfollow", base64::encode(json::to_cbor("")), client->fd));
                }
            }
        });
    });
}

