//
// Created by 박성빈 on 25. 6. 19.
//

#include "Client.h"
#include "IdGenerator.h"
#include "room/RoomServer.h"

Client::Client(const sockfd fd) : fd(fd), id(IdGenerator::generate()) {
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
