//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <set>
#include <memory>
#include "event/EventEmitter.h"
#include "socket/SocketType.h"

class RoomServer;

class Client : public EventEmitter, public std::enable_shared_from_this<Client> {
public:
    const sockfd fd;
    const std::string id;
    RoomServer* server = nullptr;
    std::set<std::string> rooms;

    explicit Client(sockfd fd);

    void join(const std::string& roomID);
    void leave(const std::string& roomID);
};

#endif //CLIENT_H
