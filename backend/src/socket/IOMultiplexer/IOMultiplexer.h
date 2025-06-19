//
// Created by 박성빈 on 25. 6. 18.
//

#ifndef IOMULTIPLEXER_H
#define IOMULTIPLEXER_H
#include <array>
#include <vector>

#include "socket/SocketType.h"

#define MAX_EVENTS 10

enum class Operation {
    ADD = 1,
    DEL = 2,
    MOD = 3
};

class IOMultiplexer {
    sockfd iofd = INVALID_FD;
    std::array<event, MAX_EVENTS> events{};
public:

    IOMultiplexer() = default;

    ~IOMultiplexer() {
        close();
    }

    IOMultiplexer(const IOMultiplexer&) = delete;
    IOMultiplexer& operator=(const IOMultiplexer&) = delete;
    IOMultiplexer(IOMultiplexer&&) = delete;
    IOMultiplexer& operator=(IOMultiplexer&&) = delete;

    void create();
    void control(Operation op, sockfd fd);
    std::vector<sockfd> wait();
    void close();
    void close(sockfd fd);
};

#endif //IOMULTIPLEXER_H
