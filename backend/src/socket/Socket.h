//
// Created by 박성빈 on 25. 6. 12.
//

#ifndef SOCKET_H
#define SOCKET_H

#include "SocketType.h"
#include <set>

enum class Method {
    READ, WRITE
};

class Socket {
    sockfd server_sockfd = INVALID_FD;

    const unsigned short port;
public:
    explicit Socket(const unsigned short port): port(port) {}

    ~Socket() {
        close();
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(Socket&&) = delete;

    void open();
    void close();
    void close(sockfd fd, const std::set<Method>& methods);
    [[nodiscard]] sockfd accept() const;

    ssize_t send(sockfd fd, const void* buf, size_t n) const;
    ssize_t recv(sockfd fd, void* buf, size_t n) const;

    [[nodiscard]] bool isOpen() const noexcept {
        return server_sockfd != INVALID_FD;
    }

    [[nodiscard]] sockfd getServerFd() const noexcept {
        return server_sockfd;
    }
private:
    bool send_all(sockfd fd, const void* buf, size_t n) const;
    bool recv_all(sockfd fd, void* buf, size_t n) const;
};

#endif //SOCKET_H
