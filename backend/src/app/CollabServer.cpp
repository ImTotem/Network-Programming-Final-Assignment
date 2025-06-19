//
// Created by 박성빈 on 25. 6. 11.
//

#include "CollabServer.h"
#include <vector>
#include <string>
#include "env/dotenv.h"
#include "log/Log.h"
#include "middleware/payload/Payload.h"
#include "middleware/protocol/Protocol.h"
#include "room/RoomServer.h"
#include "room/client/Client.h"

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
        // port = getenv("DEBUG") ? 3002 : 80;
        port = 3002;
    }

    // 3. RoomServer 엔진 생성
    io = std::make_unique<RoomServer>(port);

    // 4. 이벤트 핸들러 설정
    setupEventHandlers();

    // 5. 프로토콜 정의
    io->setProtocol(Protocol::serialize, Protocol::deserialize);
}

CollabServer::~CollabServer() = default;

void CollabServer::run() {
    Log::info("CollabServer is running...");
    io->start(); // 서버 엔진의 메인 루프 시작
}

void CollabServer::setupEventHandlers() {
    // server.on("connection", (client) => { ... });
    io->on("connection", [this](const std::vector<std::any>& args) {
        auto client = std::any_cast<std::shared_ptr<Client>>(args[0]);
        Log::info("Connection established! Client ID: {}", client->id);

        // server.to(`${client.id}`).emit("init-room");
        this->io->to(client->id).emit("init-room");

        // client.on("join-room", async (roomID) => { ... });
        client->on("join-room", [this, client](const std::vector<std::any>& room_args) {
            // 프로토콜 미들웨어에서 room_args[0]에 roomID를 string으로 파싱했다고 가정
            const auto roomID = std::any_cast<std::string>(room_args[0]);
            Log::debug("{} has joined {}", client->id, roomID);

            client->join(roomID);

            const auto sockets = this->io->socketsIn(roomID);
            if (sockets.size() <= 1) {
                this->io->to(client->id).emit("first-in-room");
            } else {
                Log::debug("{} emitting new-user to room {}", client->id, roomID);
                client->broadcast().to(roomID).emit("new-user", client->id);
            }

            std::vector<std::string> socket_ids;
            socket_ids.reserve(sockets.size());
            for (const auto& s : sockets) { socket_ids.push_back(s->id); }
            this->io->in(roomID).emit("room-user-change", socket_ids);
        });

        // client.on("server-broadcast", (roomID, encryptedData, iv) => { ... });
        client->on("server-broadcast", [client](const std::vector<std::any>& broadcast_args) {
            // 프로토콜 미들웨어에서 args를 {roomID, encryptedData, iv}로 파싱했다고 가정
            const auto& roomID = std::any_cast<const std::string&>(broadcast_args[0]);
            const auto& encryptedData = std::any_cast<const std::vector<char>&>(broadcast_args[1]);
            const auto& iv = std::any_cast<const std::vector<char>&>(broadcast_args[2]);

            Log::debug("{} sends update to {}", client->id, roomID);
            client->broadcast().to(roomID).emit("client-broadcast", encryptedData, iv);
        });

        // client.on("server-volatile-broadcast", ...);
        // TCP는 신뢰성 있는 프로토콜이므로 volatile 개념이 없음. 일반 broadcast와 동일하게 처리.
        client->on("server-volatile-broadcast", [client](const std::vector<std::any>& broadcast_args) {
            const auto& roomID = std::any_cast<const std::string&>(broadcast_args[0]);
            const auto& encryptedData = std::any_cast<const std::vector<char>&>(broadcast_args[1]);
            const auto& iv = std::any_cast<const std::vector<char>&>(broadcast_args[2]);

            Log::debug("{} sends volatile update to {}", client->id, roomID);
            client->broadcast().to(roomID).emit("client-broadcast", encryptedData, iv);
        });

        // client.on("user-follow", async (payload: OnUserFollowedPayload) => { ... });
        client->on("user-follow", [this, client](const std::vector<std::any>& follow_args) {
            // 프로토콜 미들웨어에서 args[0]을 OnUserFollowedPayload 구조체로 파싱했다고 가정
            const auto& json_string = std::any_cast<const std::string&>(follow_args[0]);
            const auto payload = nlohmann::json::parse(json_string).get<OnUserFollowedPayload>();
            const std::string roomID = "follow@" + payload.userToFollow.socketId;

            switch (payload.action) {
                case FollowAction::FOLLOW: {
                    client->join(roomID);
                    const auto sockets = this->io->socketsIn(roomID);
                    std::vector<std::string> followedBy_ids;
                    for (const auto& s : sockets) { followedBy_ids.push_back(s->id); }
                    this->io->to(payload.userToFollow.socketId).emit("user-follow-room-change", followedBy_ids);
                    break;
                }
                case FollowAction::UNFOLLOW: {
                    client->leave(roomID);
                    const auto sockets = this->io->socketsIn(roomID);
                    std::vector<std::string> followedBy_ids;
                    for (const auto& s : sockets) { followedBy_ids.push_back(s->id); }
                    this->io->to(payload.userToFollow.socketId).emit("user-follow-room-change", followedBy_ids);
                    break;
                }
            }
        });

        // client.on("disconnecting", async () => { ... });
        client->on("disconnecting", [this, client](const std::vector<std::any>& /*args*/) {
            Log::debug("{} has disconnected", client->id);
            // client->rooms는 현재 클라이언트가 속한 모든 방의 목록
            for (const auto& roomID : client->rooms) {
                const auto otherClients = this->io->socketsIn(roomID);

                const bool isFollowRoom = roomID.starts_with("follow@");

                if (!isFollowRoom && otherClients.size() > 1) {
                    std::vector<std::string> socket_ids;
                    for (const auto& c : otherClients) {
                        if (c->id != client->id) { // 나가는 자신을 제외
                            socket_ids.push_back(c->id);
                        }
                    }
                    if (!socket_ids.empty()) {
                        client->broadcast().to(roomID).emit("room-user-change", socket_ids);
                    }
                }

                if (isFollowRoom && otherClients.size() == 1 && otherClients[0]->id == client->id) {
                    // 이제 이 방에는 아무도 없게 됨 (나가는 사람 제외 시 0명)
                    const auto socketId = roomID.substr(std::string("follow@").length());
                    this->io->to(socketId).emit("broadcast-unfollow");
                }
            }
        });

    });
}

