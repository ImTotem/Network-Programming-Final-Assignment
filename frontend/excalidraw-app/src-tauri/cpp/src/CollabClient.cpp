#include "CollabClient.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

CollabClient::CollabClient() : sockfd(-1), running(false) {}

CollabClient::~CollabClient() {
    disconnect();
}

bool CollabClient::connect(const std::string& host, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        ::close(sockfd);
        sockfd = -1;
        return false;
    }

    if (::connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        ::close(sockfd);
        sockfd = -1;
        return false;
    }

    running = true;
    recvThread = std::thread(&CollabClient::recvLoop, this);
    return true;
}

void CollabClient::disconnect() {
    running = false;
    if (sockfd >= 0) {
        ::shutdown(sockfd, SHUT_RDWR);
        ::close(sockfd);
        sockfd = -1;
    }
    if (recvThread.joinable()) {
        recvThread.join();
    }
}

bool CollabClient::emit(const std::string& json_payload) {
    if (sockfd < 0) return false;
    
    // [헤더][json_payload] 형태로 전송
    // 헤더는 json_payload의 길이 (4바이트)
    uint32_t len = htonl(json_payload.size());
    if (send(sockfd, &len, sizeof(len), 0) != sizeof(len)) return false;
    if (send(sockfd, json_payload.data(), json_payload.size(), 0) != (ssize_t)json_payload.size()) return false;
    return true;
}

void CollabClient::on(const std::string& eventName, std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(handlerMutex);
    handlers[eventName] = callback;
}

void CollabClient::set_event_callback(collab_event_callback_t cb, void* user_data) {
    event_callback = cb;
    event_callback_user_data = user_data;
}

void CollabClient::recvLoop() {
    while (running) {
        // 4바이트 길이 헤더 읽기
        uint32_t len_net = 0;
        ssize_t n = recv(sockfd, &len_net, sizeof(len_net), MSG_WAITALL);
        if (n <= 0) break;
        uint32_t len = ntohl(len_net);

        std::vector<char> buffer(len);
        size_t received = 0;
        while (received < len) {
            ssize_t r = recv(sockfd, buffer.data() + received, len - received, 0);
            if (r <= 0) break;
            received += r;
        }
        if (received == len) {
            handleMessage(buffer);
        } else {
            break;
        }
    }
    running = false;
}

void CollabClient::handleMessage(const std::vector<char>& data) {
    // JSON 문자열을 그대로 콜백에 전달 (파싱하지 않음)
    std::string jsonStr(data.begin(), data.end());
    
    // 기존 C++ 핸들러 호출 (raw JSON 문자열 전달)
    {
        std::lock_guard<std::mutex> lock(handlerMutex);
        auto it = handlers.find("payload");
        if (it != handlers.end()) {
            it->second(jsonStr);
        }
    }
    
    // Rust FFI 콜백 호출 (raw JSON 문자열 전달)
    if (event_callback) {
        event_callback(jsonStr.c_str(), event_callback_user_data);
    }
}
