//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <vector>
#include <string>
#include <any>
#include <cstdint>

namespace Protocol {
    // 데이터 타입 식별을 위한 태그
    enum class Tag : uint8_t {
        EVENT_NAME = 1,
        UTF8_STRING = 2,      // 일반 문자열 (예: roomID)
        BINARY_DATA = 3,      // 바이너리 데이터 (예: encryptedData, iv)
        JSON_STRING = 4,      // JSON으로 직렬화된 객체 (예: OnUserFollowedPayload)
        STRING_VECTOR = 5,    // 문자열 벡터 (예: socket_ids)
    };

    /**
     * @brief 이벤트 이름과 인자들을 바이너리 페이로드로 직렬화합니다.
     * @param eventName 발생시킬 이벤트의 이름.
     * @param args 이벤트에 전달될 인자들의 목록.
     * @return 전송 가능한 최종 바이너리 데이터 (std::vector<char>).
     */
    std::vector<char> serialize(const std::string& eventName, const std::vector<std::any>& args);

    /**
     * @brief 수신된 바이너리 페이로드를 해석하여 이벤트 이름과 인자들로 역직렬화합니다.
     * @param raw_payload 서버로부터 수신된 원본 바이너리 데이터.
     * @param out_eventName (출력) 해석된 이벤트 이름.
     * @param out_args (출력) 해석된 인자들의 목록.
     */
    void deserialize(const std::vector<char>& raw_payload, std::string& out_eventName, std::vector<std::any>& out_args);
}

#endif //PROTOCOL_H
