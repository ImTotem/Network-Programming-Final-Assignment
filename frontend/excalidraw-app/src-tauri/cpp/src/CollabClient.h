#ifndef COLLAB_CLIENT_H
#define COLLAB_CLIENT_H

#include <string>
#include <functional>
#include <map>
#include <thread>
#include <atomic>
#include <vector>
#include <any>
#include <mutex>

// C 스타일 콜백 타입 정의
extern "C" {
    typedef void (*collab_event_callback_t)(const char* event, const char** args, int argc, void* user_data);
}

class CollabClient {
public:
    CollabClient();
    ~CollabClient();

    bool connect(const std::string& host, int port);
    void disconnect();

    // 서버로 이벤트 전송
    bool emit(const std::string& eventName, const std::vector<std::any>& args);

    // C++ 코드 내에서 직접 콜백 등록 (기존)
    void on(const std::string& eventName, std::function<void(const std::vector<std::any>&)> callback);

    // Rust FFI에서 콜백 등록 (추가)
    void set_event_callback(collab_event_callback_t cb, void* user_data);

private:
    int sockfd;
    std::thread recvThread;
    std::atomic<bool> running;
    std::map<std::string, std::function<void(const std::vector<std::any>&)>> handlers;
    std::mutex handlerMutex;

    // Rust FFI 콜백
    collab_event_callback_t event_callback = nullptr;
    void* event_callback_user_data = nullptr;

    void recvLoop();
    void handleMessage(const std::vector<char>& data);
};


#endif
