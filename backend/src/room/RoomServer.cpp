//
// Created by 박성빈 on 25. 6. 19.
//

#include "RoomServer.h"

#include "client/Client.h"
#include "log/Log.h"
#include "middleware/websocket/WebSocket.h"

RoomServer::RoomServer(const unsigned short port) : Server(port) {
    // --- Server의 저수준 이벤트를 구독하여 RoomServer의 로직 실행 ---

    on("_connection", [this](const std::vector<std::any>& args) {
        const sockfd fd = std::any_cast<sockfd>(args[0]);
        auto client = std::make_shared<Client>(fd);
        client->server = this;
        this->clients_[fd] = client;
        this->rooms_[client->id].insert(fd); // 클라이언트 고유 방 생성 및 참여

        // 고수준 "connection" 이벤트를 CollabServer에 전달
        this->emit("connection", client);
    });

    on("_disconnect", [this](const std::vector<std::any>& args) {
        const sockfd fd = std::any_cast<sockfd>(args[0]);
        if (const auto client = getClient(fd)) {
            client->emit("disconnecting");
            for(const auto& room_name : client->rooms) {
                if(this->rooms_.contains(room_name)) {
                    this->rooms_.at(room_name).erase(fd);
                }
            }
            this->clients_.erase(fd);
        }
    });

    on("_message", [this](const std::vector<std::any>& args) {
        const auto fd = std::any_cast<sockfd>(args[0]);
        const auto& payload = std::any_cast<const std::vector<char>&>(args[1]);

        if (const auto client = getClient(fd)) {
            Context context(*this, client, payload);
            for (const auto& middleware : this->pipeline_) {
                middleware(context);
                if (context.stopProcessing) return;
            }
            client->emit(context.eventName, context.args);
        }
    });

    // --- 기본 프로토콜 미들웨어 설정 ---
    // 1. WebSocket 핸드셰이크/프레임 처리 미들웨어
    use([](Context& ctx) {
        // 웹소켓이 아니면 핸드셰이크 시도
        if (!ctx.client->isWebSocket) {
            WebSocketMiddleware::handleHandshake(ctx);
            if (ctx.stopProcessing) return;
        }
        // 이미 웹소켓이면 프레임 처리
        if (ctx.client->isWebSocket) {
            WebSocketMiddleware::handleFrames(ctx);
            if (ctx.stopProcessing) return;
            // 여기서 application payload가 없으면 반드시 stopProcessing!
            if (ctx.payload.empty()) {
                ctx.stopProcessing = true;
                return;
            }
        }
        // 여기까지 왔으면 일반 TCP 패킷이거나, 웹소켓 프레임에서 실제 application payload가 추출된 상태
    });

    // 2. TLV 프로토콜 역직렬화 미들웨어 (웹소켓 handshake/프레임이 아닌 경우만)
    use([this](Context& ctx) {
        if (ctx.payload.size() < 5 || !this->deserialize_protocol_) {
            Log::warn("Protocol deserializer is not set or payload too short!");
            ctx.stopProcessing = true;
            return;
        }
        this->deserialize_protocol_(ctx.payload, ctx.eventName, ctx.args);
    });
}

void RoomServer::use(Middleware middleware) { pipeline_.push_back(std::move(middleware)); }

void RoomServer::setProtocol(
    std::function<std::vector<char>(const std::string&, const std::vector<std::any>&)> ser,
    std::function<void(const std::vector<char>&, std::string&, std::vector<std::any>&)> deser) {
    serialize_protocol_ = std::move(ser);
    deserialize_protocol_ = std::move(deser);
}

std::shared_ptr<Client> RoomServer::getClient(const sockfd fd) {
    const auto it = clients_.find(fd);
    return (it != clients_.end()) ? it->second : nullptr;
}

std::vector<sockfd> RoomServer::getAllFd() const {
    std::vector<sockfd> fds;
    fds.reserve(clients_.size());
    for(const auto& pair : clients_) {
        fds.push_back(pair.first);
    }
    return fds;
}

void RoomServer::joinRoom(const sockfd fd, const std::string& roomID) { rooms_[roomID].insert(fd); }
void RoomServer::leaveRoom(const sockfd fd, const std::string& roomID) { if (rooms_.contains(roomID)) { rooms_[roomID].erase(fd); } }

std::vector<std::shared_ptr<Client>> RoomServer::socketsIn(const std::string& roomID) {
    std::vector<std::shared_ptr<Client>> clients_in_room;
    if (rooms_.contains(roomID)) {
        clients_in_room.reserve(rooms_.at(roomID).size());
        for (const sockfd fd : rooms_.at(roomID)) {
            if (auto client = getClient(fd)) {
                clients_in_room.push_back(client);
            }
        }
    }
    return clients_in_room;
}

// === Helper 클래스들을 반환하는 진입점 메서드 ===

RoomServer::ToHelper RoomServer::to(const std::string& roomID) { return ToHelper(*this, roomID); }
RoomServer::ToHelper RoomServer::in(const std::string& roomID) { return to(roomID); }
RoomServer::BroadcastHelper RoomServer::broadcast(const sockfd senderFd) { return BroadcastHelper(*this, senderFd); }


// === Helper 클래스들의 emit 구현 (템플릿이므로 헤더에 두는 것이 일반적이나, 여기에 명시) ===

RoomServer::ToHelper::ToHelper(RoomServer& server, std::string room) : server_(server), room_id_(std::move(room)) {}

RoomServer::BroadcastHelper::BroadcastHelper(RoomServer& server, const sockfd senderFd) : server_(server), sender_fd_(senderFd) {}

RoomServer::BroadcastHelper& RoomServer::BroadcastHelper::to(const std::string& room) {
    room_id_ = room;
    return *this;
}

// --- 명시적 템플릿 인스턴스화 ---
// 사용하는 타입이 많아지면 여기에 추가해야 할 수 있습니다. 지금은 생략합니다.
// 예: template void RoomServer::ToHelper::emit<std::string>(const std::string&, std::string);
