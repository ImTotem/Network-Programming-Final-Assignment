//
// Created by 박성빈 on 25. 6. 12.
//

#include "Server.h"

#include <cstring>
#include <unistd.h>

#include "log/Log.h"
#include "socket/IOMultiplexer/IOMultiplexingException.h"
#include "utils/endian.h"

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

    client_buffers[client_fd];

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
    client_buffers.erase(fd);
    io->control(Operation::DEL, fd);
    socket->close(fd, {Method::READ, Method::WRITE});
    Log::info("Connection closed for fd: {}", fd);
    emit("_disconnect", Packet("disconnect", "", fd));
}

void Server::send(const Packet& packet) {
    // [헤더(=JSON길이)][JSON]
    const std::string raw = serializePacket(packet.event, packet.data);

    // 1. 헤더 생성 (JSON 데이터의 길이)
    uint32_t header = htonl(raw.size());

    std::vector<char> full_packet;
    full_packet.insert(full_packet.end(), reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(header));
    full_packet.insert(full_packet.end(), raw.begin(), raw.end());

    if (socket->send(packet.fd, full_packet.data(), full_packet.size()) < 0) {
        Log::warn("Failed to send packet to fd: {}", packet.fd);
    }
    Log::debug("Send packet: {} {} {}", packet.event, packet.data, packet.fd);
}

void Server::recv(const sockfd fd) {
    const auto it = client_buffers.find(fd);

    PacketBuffer& packet_buffer = it->second;

    const size_t remaining = packet_buffer.remainingSize();

    // 2. 이번에 실제로 읽을 크기를 결정합니다. (최대 4096까지만)
    const size_t bytes_to_read = std::min(static_cast<size_t>(4096), (remaining));

    // 읽어야 할 데이터가 없으면 아무것도 하지 않습니다.
    if (bytes_to_read == 0) {
        return;
    }

    // 3. 정확히 필요한 크기만큼만 임시 버퍼(벡터)를 생성합니다.
    std::vector<char> temp_buffer(bytes_to_read);

    // 4. recv()를 딱 한 번 호출하고, 결과를 안전하게 ssize_t에 저장합니다.
    const ssize_t bytes_actually_read = socket->recv(fd, temp_buffer.data(), bytes_to_read);

    // 5. 에러 또는 연결 종료를 처리합니다.
    if (bytes_actually_read <= 0) {
        close(fd);
        return;
    }

    // 6. 읽은 만큼만 PacketBuffer에 데이터를 추가합니다.
    packet_buffer.append(temp_buffer.data(), static_cast<size_t>(bytes_actually_read));

    if (packet_buffer.isReady()) {
        emit("_message", packet_buffer.toPacket(fd));
        packet_buffer.reset();
    }
}
