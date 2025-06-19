//
// Created by 박성빈 on 25. 6. 19.
//

// src/room/Client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <set>
#include <memory>
#include "event/EventEmitter.h"
#include "room/RoomServer.h"
#include "socket/SocketType.h"

class Client : public EventEmitter, public std::enable_shared_from_this<Client> {
public:
    const sockfd fd;
    const std::string id;

    RoomServer* server = nullptr;
    std::set<std::string> rooms;

    // --- WebSocket 상태 추가 ---
    bool isWebSocket = false;
    // 조각난 웹소켓 프레임을 모으기 위한 전용 버퍼
    std::vector<char> websocket_buffer;

    explicit Client(sockfd fd);

    void join(const std::string& roomID);
    void leave(const std::string& roomID);

    // BroadcastHelper를 직접 참조하는 대신 RoomServer에 위임합니다.
    // 이렇게 하면 Client.h가 BroadcastHelper의 정의를 알 필요가 없습니다.
    RoomServer::BroadcastHelper broadcast(); // 반환 타입을 auto로 변경
};

#endif //CLIENT_H
