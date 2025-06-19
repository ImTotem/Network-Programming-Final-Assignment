//
// Created by 박성빈 on 25. 6. 12.
//

#ifndef EVENTEMITTER_H
#define EVENTEMITTER_H
#include <functional>
#include <string>
#include <any>
#include <map>

class EventEmitter {
public:
    using Listener = std::function<void(const std::vector<std::any>&)>;

    virtual ~EventEmitter() = default;

    EventEmitter(const EventEmitter&) = delete;
    EventEmitter& operator=(const EventEmitter&) = delete;
    EventEmitter(EventEmitter&&) = delete;
    EventEmitter& operator=(EventEmitter&&) = delete;

    /**
     * @brief 특정 이벤트에 대한 리스너를 등록합니다.
     * @param eventName 구독할 이벤트의 이름.
     * @param listener 이벤트 발생 시 호출될 콜백 함수.
     */
    void on(const std::string& eventName, Listener listener) {
        listeners[eventName].push_back(std::move(listener));
    }

    /**
     * @brief 특정 이벤트를 발생시켜 모든 리스너를 실행합니다.
     * @tparam Args 리스너에게 전달될 인자들의 가변 템플릿 타입.
     * @param eventName 발생시킬 이벤트의 이름.
     * @param args 리스너에게 전달될 인자들.
     */
    template<typename... Args>
    void emit(const std::string& eventName, Args&&... args) {
        if (
            const auto it = listeners.find(eventName);
            it != listeners.end()
        ) {
            // 인자들을 std::vector<std::any>로 패킹합니다.
            std::vector<std::any> arguments;
            // 폴드 표현식(C++17)을 사용하여 모든 인자를 arguments 벡터에 추가합니다.
            (arguments.push_back(std::any(std::forward<Args>(args))), ...);

            // 해당 이벤트에 등록된 모든 리스너를 순회하며 실행합니다.
            for (const auto& listener : it->second) {
                listener(arguments);
            }
        }
    }
private:
    std::map<std::string, std::vector<Listener>, std::less<>> listeners;
protected:
    EventEmitter() = default;
};


#endif //EVENTEMITTER_H
