//
// Created by 박성빈 on 25. 6. 19.
//

#include "Client.h"
#include "IdGenerator.h"
#include "room/RoomServer.h"

Client::Client(const sockfd fd) : fd(fd), id(IdGenerator::generate()) {
    // Socket.IO처럼, 모든 클라이언트는 자기 ID와 동일한 이름의 방에 자동으로 참여합니다.
    rooms.insert(id);
}

void Client::join(const std::string& roomID) {
    rooms.insert(roomID);
    if (server) {
        server->joinRoom(this->fd, roomID);
    }
}

void Client::leave(const std::string& roomID) {
    rooms.erase(roomID);
    if (server) {
        server->leaveRoom(this->fd, roomID);
    }
}

RoomServer::BroadcastHelper Client::broadcast() {
    if (server) {
        return server->broadcast(this->fd);
    }
    throw std::runtime_error("Client is not attached to a server.");
}
