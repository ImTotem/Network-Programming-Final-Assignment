//
// Created by 박성빈 on 25. 6. 20.
//

#ifndef PACKET_H
#define PACKET_H
#include <string>

#include "socket/SocketType.h"

struct Packet {
    std::string event;
    std::string data; // base64 string
    sockfd fd;
    Packet(std::string event, std::string data, const sockfd fd) : event(std::move(event)), data(std::move(data)), fd(fd) {}
};

#endif //PACKET_H
