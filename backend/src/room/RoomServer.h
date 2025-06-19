//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef ROOMSERVER_H
#define ROOMSERVER_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include "server/Server.h"
#include "middleware/Middleware.h"

class RoomServer : public Server {
public:
    explicit RoomServer(unsigned short port);

    // 미들웨어 및 프로토콜 설정
    void use(Middleware middleware);
    void setProtocol(
        std::function<std::vector<char>(const std::string&, const std::vector<std::any>&)> serializer,
        std::function<void(const std::vector<char>&, std::string&, std::vector<std::any>&)> deserializer
    );

    // === Socket.IO API 모방 ===
    // 내부 헬퍼 클래스 선언
    class ToHelper;
    class BroadcastHelper;

    ToHelper to(const std::string& roomID);
    ToHelper in(const std::string& roomID); // to의 별칭
    BroadcastHelper broadcast(int senderFd);

    // Room 및 Client 관리
    void joinRoom(int fd, const std::string& roomID);
    void leaveRoom(int fd, const std::string& roomID);
    std::vector<std::shared_ptr<Client>> socketsIn(const std::string& roomID);
    std::shared_ptr<Client> getClient(int fd);

    // stop()이 모든 클라이언트를 닫을 수 있도록 getAllFd 오버라이드
    [[nodiscard]] std::vector<int> getAllFd() const override;

private:
    std::vector<Middleware> pipeline_;
    std::function<std::vector<char>(const std::string&, const std::vector<std::any>&)> serialize_protocol_;
    std::function<void(const std::vector<char>&, std::string&, std::vector<std::any>&)> deserialize_protocol_;

    std::map<int, std::shared_ptr<Client>> clients_;
    std::map<std::string, std::set<int>> rooms_;
};

// --- 헬퍼 클래스 정의 ---
class RoomServer::ToHelper {
public:
    ToHelper(RoomServer& server, std::string room);
    template<typename... Args>
    void emit(const std::string& eventName, Args... args) {
        if (server_.serialize_protocol_) {
            std::vector<std::any> arguments;
            (arguments.push_back(std::any(std::forward<Args>(args))), ...);
            const std::vector<char> payload = server_.serialize_protocol_(eventName, arguments);

            if (server_.rooms_.contains(room_id_)) {
                for (const sockfd fd : server_.rooms_.at(room_id_)) {
                    server_.write(fd, payload);
                }
            }
        }
    }
private:
    RoomServer& server_;
    std::string room_id_;
};

class RoomServer::BroadcastHelper {
public:
    BroadcastHelper(RoomServer& server, int senderFd);
    BroadcastHelper& to(const std::string& room);
    template<typename... Args>
    void emit(const std::string& eventName, Args... args) {
        if (server_.serialize_protocol_) {
            std::vector<std::any> arguments;
            (arguments.push_back(std::any(std::forward<Args>(args))), ...);
            const std::vector<char> payload = server_.serialize_protocol_(eventName, arguments);

            // room_id_가 비어있으면 모든 클라이언트에게 브로드캐스트 (socket.io와 다름, 필요 시 구현)
            if (server_.rooms_.contains(room_id_)) {
                for (const sockfd fd : server_.rooms_.at(room_id_)) {
                    if (fd != sender_fd_) {
                        server_.write(fd, payload);
                    }
                }
            }
        }
    }
private:
    RoomServer& server_;
    int sender_fd_;
    std::string room_id_;
};
#endif //ROOMSERVER_H

