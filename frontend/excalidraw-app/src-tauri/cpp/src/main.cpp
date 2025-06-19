#include "CollabClient.h"
#include <iostream>
#include <thread>

int main() {
    CollabClient client;
    if (!client.connect("10.211.55.9", 3002)) {
        std::cerr << "서버 연결 실패" << std::endl;
        return 1;
    }

    // 서버에서 "init-room" 이벤트 수신 시 처리
    client.on("init-room", [](const std::vector<std::any>& args) {
        std::cout << "방 초기화됨!" << std::endl;
    });

    // 방 입장
    client.emit("join-room", {std::string("room123")});

    // 예시: 10초 후 종료
    std::this_thread::sleep_for(std::chrono::seconds(10));
    client.disconnect();
    return 0;
} 