#include "CollabClient.h"
#include "protocol/Protocol.h"

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

bool CollabClient::emit(const std::string& eventName, const std::vector<std::any>& args) {
    if (sockfd < 0) return false;
    // 항상 eventName은 "payload"로, args는 JSON string 하나만
    std::string fixedEvent = "payload";
    std::vector<std::any> fixedArgs;
    if (!args.empty() && args[0].type() == typeid(std::string)) {
        fixedArgs.push_back(std::any_cast<std::string>(args[0]));
    } else {
        // 잘못된 사용: 빈 payload
        fixedArgs.push_back(std::string("{\"event\":\"unknown\",\"data\":\"\"}"));
    }
    std::vector<char> payload = Protocol::serialize(fixedEvent, fixedArgs);

    // [길이(4바이트)][payload]
    uint32_t len = htonl(payload.size());
    if (::send(sockfd, &len, sizeof(len), 0) != sizeof(len)) return false;
    if (!payload.empty() && ::send(sockfd, payload.data(), payload.size(), 0) != (ssize_t)payload.size()) return false;
    return true;
}

void CollabClient::on(const std::string& eventName, std::function<void(const std::vector<std::any>&)> callback) {
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
        if (len == 0 || len > 10 * 1024 * 1024) break; // 10MB 이상은 비정상

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
    std::string eventName;
    std::vector<std::any> args;
    Protocol::deserialize(data, eventName, args);

    // 항상 eventName은 "payload", args[0]은 JSON string임을 가정
    if (event_callback && !args.empty() && args[0].type() == typeid(std::string)) {
        std::string jsonStr = std::any_cast<std::string>(args[0]);
        // nlohmann::json j = nlohmann::json::parse(jsonStr);
        // std::string event = j["event"];
        // std::string data = j["data"];
        // std::string decoded = base64::from_base64(data);
        // ...
    }
    // 기존 C++ 핸들러도 동일하게 동작
    {
        std::lock_guard<std::mutex> lock(handlerMutex);
        auto it = handlers.find(eventName);
        if (it != handlers.end()) {
            it->second(args);
        }
    }
    // Rust FFI 콜백도 동일하게 전달
    if (event_callback) {
        std::vector<std::string> str_args;
        for (const auto& arg : args) {
            if (arg.type() == typeid(std::string)) {
                str_args.push_back(std::any_cast<std::string>(arg));
            } else {
                str_args.push_back("");
            }
        }
        std::vector<const char*> c_args;
        for (const auto& s : str_args) c_args.push_back(s.c_str());
        event_callback(eventName.c_str(), c_args.data(), c_args.size(), event_callback_user_data);
    }
}
