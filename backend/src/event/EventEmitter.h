//
// Created by 박성빈 on 25. 6. 12.
//

#ifndef EVENTEMITTER_H
#define EVENTEMITTER_H
#include <functional>
#include <string>
#include <map>

class Packet;

class EventEmitter {
public:
    using Listener = std::function<void(const Packet&)>;

    virtual ~EventEmitter() = default;

    EventEmitter(const EventEmitter&) = delete;
    EventEmitter& operator=(const EventEmitter&) = delete;
    EventEmitter(EventEmitter&&) = delete;
    EventEmitter& operator=(EventEmitter&&) = delete;

    /**
     * @brief 특정 이벤트에 대한 리스너를 등록합니다.
     * @param event 구독할 이벤트의 이름.
     * @param listener 이벤트 발생 시 호출될 콜백 함수.
     */
    void on(const std::string& event, Listener listener) {
        listeners[event].push_back(std::move(listener));
    }

    /**
     * @brief 특정 이벤트를 발생시켜 모든 리스너를 실행합니다.
     * @param event 발생시킬 이벤트의 이름.
     * @param arg 리스너에게 전달될 인자.
     */
    void emit(const std::string& event, const Packet& arg) {
        if (const auto it = listeners.find(event); it != listeners.end()) {
            for (const auto& listener : it->second) {
                listener(arg);
            }
        }
    }
private:
    std::map<std::string, std::vector<Listener>> listeners;
protected:
    EventEmitter() = default;
};


#endif //EVENTEMITTER_H
