//
// Created by 박성빈 on 25. 6. 19.
//

#include "RoomServer.h"

#include <ranges>

#include "client/Client.h"
#include <utility>

#include "utils/base64.h"

RoomServer::RoomServer(const unsigned short port) : Server(port) {
    on("_connection", [this](const Packet& packet) {
        auto client = std::make_shared<Client>(packet.fd);
        client->server = this;
        clients[packet.fd] = client;
        rooms[client->id].insert(packet.fd);
        emit(packet.event, Packet(packet.event, client->id, packet.fd));
    });

    on("_disconnect", [this](const Packet& packet) {
        if (const auto client = getClient(packet.fd)) {
            client->server->send(packet);
            for(const auto& room_name : client->rooms) {
                auto it = rooms.find(room_name);
                if(it != rooms.end()) {
                    it->second.erase(packet.fd);
                }
            }
            clients.erase(packet.fd);
        }
    });

    on("_message", [this](const Packet& packet) {
        if (const auto client = getClient(packet.fd)) {
            client->emit(packet.event, packet);
        }
    });
}

std::shared_ptr<Client> RoomServer::getClient(const sockfd fd) {
    const auto it = clients.find(fd);
    return (it != clients.end()) ? it->second : nullptr;
}

std::vector<sockfd> RoomServer::getAllFd() const {
    std::vector<sockfd> fds;
    fds.reserve(clients.size());
    for(const auto &key: clients | std::views::keys) {
        fds.push_back(key);
    }
    return fds;
}

void RoomServer::joinRoom(const sockfd fd, const std::string& roomID) {
    rooms[roomID].insert(fd);
}

void RoomServer::leaveRoom(const sockfd fd, const std::string& roomID) {
    auto it = rooms.find(roomID);
    if (it != rooms.end()) {
        it->second.erase(fd);
    }
}

std::vector<std::shared_ptr<Client>> RoomServer::socketsIn(const std::string& roomID) {
    std::vector<std::shared_ptr<Client>> clients_in_room;
    auto it = rooms.find(roomID);
    if (it != rooms.end()) {
        clients_in_room.reserve(it->second.size());
        for (const sockfd fd : it->second) {
            if (auto client = getClient(fd)) {
                clients_in_room.push_back(client);
            }
        }
    }
    return clients_in_room;
}

void RoomServer::emitToRoom(const std::string& roomID, const Packet& packet) {
    auto it = rooms.find(roomID);
    if (it != rooms.end()) {
        for (const sockfd fd : it->second) {
            if (const auto client = getClient(fd)) {
                client->server->send(Packet(packet.event, packet.data, client->fd));
            }
        }
    }
}

void RoomServer::broadcastToRoom(const std::string& roomID, const Packet& packet) {
    if (rooms.contains(roomID)) {
        for (const sockfd fd : rooms[roomID]) {
            if (fd != packet.fd) {
                if (const auto client = getClient(fd)) {
                    client->server->send(Packet(packet.event, packet.data, client->fd));
                }
            }
        }
    }
}

std::shared_ptr<Client> RoomServer::getClientById(const std::string& id) {
    for (const auto &client: clients | std::views::values) {
        if (client && client->id == id) return client;
    }
    return nullptr;
}
