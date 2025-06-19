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
    std::vector<char> payload = Protocol::serialize(eventName, args);

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

    // 1. C++ 핸들러 우선 호출
    {
        std::lock_guard<std::mutex> lock(handlerMutex);
        auto it = handlers.find(eventName);
        if (it != handlers.end()) {
            it->second(args);
        }
    }

    // 2. Rust FFI 콜백이 등록되어 있으면 호출
    if (event_callback) {
        // std::any -> std::string 변환 (모든 인자를 string으로 전달)
        std::vector<std::string> str_args;
        for (const auto& arg : args) {
            if (arg.type() == typeid(std::string)) {
                str_args.push_back(std::any_cast<std::string>(arg));
            } else if (arg.type() == typeid(std::vector<char>)) {
                // 바이너리 데이터는 base64 등으로 변환 필요할 수 있음. 여기선 string으로 변환 시도
                const auto& bin = std::any_cast<std::vector<char>>(arg);
                str_args.push_back(std::string(bin.begin(), bin.end()));
            } else if (arg.type() == typeid(std::vector<std::string>)) {
                // 벡터는 콤마로 join (실제 사용에 맞게 수정 필요)
                const auto& vec = std::any_cast<std::vector<std::string>>(arg);
                std::string joined;
                for (size_t i = 0; i < vec.size(); ++i) {
                    joined += vec[i];
                    if (i + 1 < vec.size()) joined += ",";
                }
                str_args.push_back(joined);
            } else {
                str_args.push_back("[unsupported type]");
            }
        }
        // C 스타일 배열로 변환
        std::vector<const char*> c_args;
        for (const auto& s : str_args) c_args.push_back(s.c_str());
        event_callback(eventName.c_str(), c_args.data(), c_args.size(), event_callback_user_data);
    }
}
