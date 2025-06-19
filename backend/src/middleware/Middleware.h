// src/middleware/Middleware.h
#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include <functional>
#include <vector>
#include <string>
#include <any>
#include <memory>
// #include "room/Client.h" // 이 줄을 삭제!

// 전방 선언으로 순환 참조를 끊습니다.
class RoomServer;
class Client;

// 미들웨어를 통해 전달될 데이터 컨텍스트
struct Context {
    RoomServer& server;
    std::shared_ptr<Client> client; // 이제 Client가 불완전한 타입이어도 OK
    std::vector<char> payload;

    std::string eventName;
    std::vector<std::any> args;
    bool stopProcessing = false;

    // 생성자는 cpp 파일로 옮겨서 완전한 타입을 사용할 수 있게 합니다.
    Context(RoomServer& srv, std::shared_ptr<Client> cli, const std::vector<char>& raw_payload)
        : server(srv), client(std::move(cli)), payload(raw_payload) {}
};

using Middleware = std::function<void(Context&)>;

#endif //MIDDLEWARE_H
