//
// Created by 박성빈 on 25. 6. 12.
//

#include "socket/Socket.h"

#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "SocketException.h"
#include "log/Log.h"

void Socket::open() {
    if (isOpen()) {
        return;
    }

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 소켓 생성
    if (!isOpen()) {
        Log::error<SocketCreationException>(errno ? strerror(errno) : "unknown error");
    }

    // SO_REUSEADDR 옵션 설정
    if (
        constexpr int option_val = 1;
        setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &option_val, sizeof(option_val)) < 0
    ) {
        close();
        Log::error<SocketOptionException>("SO_REUSEADDR failed: {}", errno ? strerror(errno) : "unknown error");
    }

    sockaddr_in sin{};
    memset(&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_sockfd, std::bit_cast<sockaddr *>(&sin), sizeof(sin)) < 0) {
        close();
        Log::error<SocketBindException>("port: {}, {}", port, errno ? strerror(errno) : "unknown error");
    }

    if (
        constexpr int backlog = 128;
        listen(server_sockfd, backlog) < 0
    ) {
        close();
        Log::error<SocketListenException>(errno ? strerror(errno) : "unknown error");
    }

    Log::info("Server listening on port {}", port);
}

void Socket::close() {
    if (server_sockfd == INVALID_FD) {
        return;
    }

    if (::close(server_sockfd) < 0) {
        Log::error<SocketCloseException>(errno ? strerror(errno) : "unknown error");
    }

    server_sockfd = INVALID_FD;
}

void Socket::close(const sockfd fd, const std::set<Method>& methods) {
    for (Method method : methods) {
        shutdown(fd, static_cast<int>(method));
    }
}


sockfd Socket::accept() const {
    sockaddr_in cli_addr{};
    socklen_t clientlen = sizeof(cli_addr);

    const sockfd client_sock = ::accept(server_sockfd, std::bit_cast<struct sockaddr *>(&cli_addr), &clientlen);
    if (client_sock < 0) {
        // Handle accept errors, especially for non-blocking accept (though this is blocking)
        return INVALID_FD;
    }

    // --- 소켓 keepalive 및 타임아웃 설정 (4시간) ---
    int keepalive = 1;
    setsockopt(client_sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    int keepidle = 14400; // 4시간
    int keepintvl = 60;   // 1분마다 probe
    int keepcnt = 5;      // 5번 실패 시 끊음
    setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(client_sock, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
    // --- 끝 ---


    return client_sock;
}

bool Socket::send_all(const sockfd fd, const void *buf, const size_t n) const {
    size_t total_sent = 0;
    const auto p = static_cast<const char*>(buf);
    while (total_sent < n) {
        const ssize_t sent = ::send(fd, p + total_sent, n - total_sent, 0);
        if (sent <= 0) {
            return false;
        }
        total_sent += sent;
    }
    return true;
}

bool Socket::recv_all(const sockfd fd, void *buf, const size_t n) const {
    size_t total_received = 0;
    const auto p = static_cast<char*>(buf);
    while (total_received < n) {
        const ssize_t received = ::recv(fd, p + total_received, n - total_received, 0);
        if (received <= 0) {
            // 0은 정상 종료, -1은 에러. 모두 실패로 간주.
            return false;
        }
        total_received += received;
    }
    return true;
}

ssize_t Socket::send(sockfd fd, const void* buf, size_t n) const {
    size_t total_sent = 0;
    const auto p = static_cast<const char*>(buf);
    while (total_sent < n) {
        // MSG_NOSIGNAL: SIGPIPE 시그널 방지
        const ssize_t sent = ::send(fd, p + total_sent, n - total_sent, MSG_NOSIGNAL);
        if (sent <= 0) {
            // 0은 비정상, -1은 에러. 모두 실패로 간주.
            return -1;
        }
        total_sent += sent;
    }
    return total_sent;
}

ssize_t Socket::recv(sockfd fd, void* buf, size_t n) const {
    return ::recv(fd, buf, n, 0);
}
