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

#include "client/Client.h"
#include "server/Server.h"
#include "server/Packet.h"

class RoomServer : public Server {
public:
    explicit RoomServer(unsigned short port);

    void joinRoom(sockfd fd, const std::string& roomID);
    void leaveRoom(sockfd fd, const std::string& roomID);
    void emitToRoom(const std::string& roomID, const Packet& packet);
    void broadcastToRoom(const std::string& roomID, const Packet& packet);
    std::vector<std::shared_ptr<Client>> socketsIn(const std::string& roomID);
    std::shared_ptr<Client> getClient(sockfd fd);
    std::shared_ptr<Client> getClientById(const std::string& id);
    [[nodiscard]] std::vector<sockfd> getAllFd() const override;

private:
    std::map<sockfd, std::shared_ptr<Client>> clients;
    std::map<std::string, std::set<sockfd>> rooms;
};

#endif //ROOMSERVER_H

