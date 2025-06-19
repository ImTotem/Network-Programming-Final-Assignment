//
// Created by 박성빈 on 25. 6. 12.
//

#include "Server.h"

#include <cstring>
#include <unistd.h>

#include "log/Log.h"
#include "socket/IOMultiplexer/IOMultiplexingException.h"
#include "utils/Endian.h"

[[noreturn]] void Server::start() {
    socket->open();
    io->create();

    io->control(Operation::ADD, socket->getServerFd());
    io->control(Operation::ADD, STDIN_FILENO);

    while (true) {
        for (const sockfd current_fd : io->wait()) {
            if (current_fd == socket->getServerFd()) {
                handleNewConnection();
            } else if (current_fd == STDIN_FILENO) {
                handleStdinInput();
            } else {
                read(current_fd);
            }
        }
    }
}

void Server::stop() {
    Log::info("Server shutting down (EOF on stdin).");
    // Graceful shutdown sequence
    socket->close();  // Stop accepting new connections
    for (const auto fd : getAllFd()) {
        close(fd);
    }
    io->close();
    exit(EXIT_SUCCESS);
}

void Server::handleNewConnection() {
    const int client_fd = socket->accept();
    if (client_fd <= INVALID_FD) {
        // Handle accept errors, especially for non-blocking accept (though this is blocking)
        Log::warn("Failed to accept new connection");
        return;
    }

    io->control(Operation::ADD, client_fd);
    emit("_connection", client_fd);
    Log::info("New connection established, fd: {}", client_fd);
}

void Server::handleStdinInput() {
    std::array<char, 4096> buffer{};
    if (fgets(buffer.data(), sizeof(buffer) - 1, stdin) == nullptr) {
        stop();
    }
}

void Server::close(int fd) {
    if (client_buffers.contains(fd)) {
        emit("_disconnect", fd);
        io->control(Operation::DEL, fd);
        socket->close(fd, {Method::READ, Method::WRITE});
        client_buffers.erase(fd);
        Log::info("Connection closed for fd: {}", fd);
    }
}

void Server::read(int clientFd) {
    ssize_t length = socket->recv(clientFd, recv_buffer.data(), recv_buffer.size());

    if (length > 0) {
        auto& buffer = client_buffers[clientFd];
        buffer.insert(buffer.end(), recv_buffer.begin(), recv_buffer.begin() + length);

        // 길이 헤더 기반의 TCP 프레이밍은 이제 WebSocket 미들웨어가 아닌,
        // 일반 TCP 통신에서만 사용되어야 함. 이 부분은 미들웨어 구조에서 다시 결정.
        // 지금은 받은 데이터를 그대로 "message" 이벤트로 올립니다.
        if (!buffer.empty()) {
            emit("_message", clientFd, std::vector(buffer.begin(), buffer.end()));
        }
        buffer.clear(); // 처리했으므로 버퍼 비움
    } else {
        close(clientFd);
    }
}

// 이 함수는 [길이][데이터] 프레임을 만들어 보냅니다. (일반 TCP용)
void Server::write(int fd, const std::vector<char>& data) {
    uint32_t len_net = Endian::hostToNetwork32(data.size());
    socket->send(fd, &len_net, sizeof(len_net));
    if (!data.empty()) {
        socket->send(fd, data.data(), data.size());
    }
}

// 이 함수는 데이터를 있는 그대로 보냅니다. (WebSocket 핸드셰이크용)
void Server::write_raw(int fd, const std::string& data) {
    socket->send(fd, data.data(), data.size());
}

