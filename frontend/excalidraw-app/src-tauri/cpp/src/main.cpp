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
        if (!args.empty() && args[0].type() == typeid(std::string)) {
            std::string base64data = std::any_cast<std::string>(args[0]);
            // base64 디코딩 함수 필요 (base64::from_base64)
            // std::string decoded = base64::from_base64(base64data);
            std::cout << "방 초기화됨! (base64data: " << base64data << ")" << std::endl;
        }
    });

    // 방 입장
    std::string roomId = "room123";
    // base64 인코딩 함수 필요 (base64::to_base64)
    // std::string encoded = base64::to_base64(roomId);
    client.emit("join-room", {roomId});

    // 예시: 10초 후 종료
    std::this_thread::sleep_for(std::chrono::seconds(10));
    client.disconnect();
    return 0;
}