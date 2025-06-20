//
// Created by 박성빈 on 25. 6. 12.
//

#include "Server.h"

#include <cstring>
#include <unistd.h>

#include "log/Log.h"
#include "socket/IOMultiplexer/IOMultiplexingException.h"

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
                recv(current_fd);
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
    const sockfd client_fd = socket->accept();
    if (client_fd <= INVALID_FD) {
        Log::warn("Failed to accept new connection");
        return;
    }
    io->control(Operation::ADD, client_fd);
    emit("_connection", Packet("connection", "", client_fd));
    Log::info("New connection established, fd: {}", client_fd);
}

void Server::handleStdinInput() {
    std::array<char, 4096> buffer{};
    if (fgets(buffer.data(), sizeof(buffer) - 1, stdin) == nullptr) {
        stop();
    }
}

void Server::close(const sockfd fd) {
    io->control(Operation::DEL, fd);
    socket->close(fd, {Method::READ, Method::WRITE});
    Log::info("Connection closed for fd: {}", fd);
    emit("_disconnect", Packet("disconnect", "", fd));
}

void Server::send(const Packet& packet) {
    const std::string raw = serializePacket(packet.event, packet.data);
    socket->send(packet.fd, raw.data(), raw.size());
}

void Server::recv(const sockfd fd) {
    if (const ssize_t length = socket->recv(fd, recv_buffer.data(), recv_buffer.size()); length > 0) {
        const std::string payload(recv_buffer.begin(), recv_buffer.begin() + length);
        auto [event, data] = parsePacket(payload);
        const Packet packet(event, data, fd);
        Log::debug("{} {} {}", packet.event, packet.data, packet.fd);
        emit("_message", packet);
    } else {
        close(fd);
    }
}
