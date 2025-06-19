//
// Created by 박성빈 on 25. 6. 19.
//

#include "Protocol.h"
#include <cstring>
#include <stdexcept>
#include "app/CollabServer.h"
#include "log/Log.h"
#include "middleware/payload/Payload.h"

namespace Protocol {
    void append_bytes(std::vector<char>& buffer, const void* data, const size_t size) {
        buffer.insert(buffer.end(), static_cast<const char*>(data), static_cast<const char*>(data) + size);
    }

    void append_u32(std::vector<char>& buffer, const uint32_t value) {
        // 호스트 바이트 순서 그대로 쓴다. 변환은 Socket 계층의 몫.
        append_bytes(buffer, &value, sizeof(value));
    }

    uint32_t read_u32(const char* buffer, size_t& offset) {
        uint32_t value;
        memcpy(&value, buffer + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        // 호스트 바이트 순서 그대로 읽는다. 변환은 Socket 계층에서 이미 처리됨.
        return value;
    }

    // 값(Value)을 직렬화하는 템플릿 헬퍼
    template<typename T>
    std::vector<char> create_value_buffer(const T&);

    // string에 대한 특수화
    template<> std::vector<char> create_value_buffer<std::string>(const std::string& value) {
        std::vector<char> buffer;
        append_u32(buffer, value.length());
        append_bytes(buffer, value.data(), value.length());
        return buffer;
    }

    // vector<char>에 대한 특수화
    template<> std::vector<char> create_value_buffer<std::vector<char>>(const std::vector<char>& value) {
        return value; // 바이너리는 길이 정보 없이 데이터 자체가 값
    }

    // vector<string>에 대한 특수화
    template<> std::vector<char> create_value_buffer<std::vector<std::string>>(const std::vector<std::string>& value) {
        std::vector<char> buffer;
        append_u32(buffer, value.size()); // 벡터 크기
        for(const auto& str : value) {
            append_u32(buffer, str.length());
            append_bytes(buffer, str.data(), str.length());
        }
        return buffer;
    }

    // Type-Length-Value 구조를 만드는 메인 헬퍼
    void append_tlv(std::vector<char>& buffer, Tag tag, const std::vector<char>& value_buffer) {
        append_bytes(buffer, &tag, 1);
        append_u32(buffer, value_buffer.size());
        append_bytes(buffer, value_buffer.data(), value_buffer.size());
    }

    // --- 직렬화 메인 함수 ---
    std::vector<char> serialize(const std::string& eventName, const std::vector<std::any>& args) {
        std::vector<char> buffer;

        try {
            // 1. 이벤트 이름 직렬화
            append_tlv(buffer, Tag::EVENT_NAME, create_value_buffer(eventName));

            // 2. 인자들 직렬화
            for (const auto& arg : args) {
                Log::debug("Protocol::serialize - arg type: {}", arg.type().name());
                
                if (arg.type() == typeid(std::string)) {
                    append_tlv(buffer, Tag::UTF8_STRING, create_value_buffer(std::any_cast<std::string>(arg)));
                } else if (arg.type() == typeid(std::vector<char>)) {
                    append_tlv(buffer, Tag::BINARY_DATA, create_value_buffer(std::any_cast<std::vector<char>>(arg)));
                } else if (arg.type() == typeid(std::vector<std::string>)) {
                    append_tlv(buffer, Tag::STRING_VECTOR, create_value_buffer(std::any_cast<std::vector<std::string>>(arg)));
                } else if (arg.type() == typeid(OnUserFollowedPayload)) {
                    nlohmann::json j = std::any_cast<OnUserFollowedPayload>(arg);
                    append_tlv(buffer, Tag::JSON_STRING, create_value_buffer(j.dump()));
                } else {
                    Log::warn("Protocol::serialize - Unsupported type: {}", arg.type().name());
                    // 지원하지 않는 타입은 JSON으로 변환 시도
                    try {
                        nlohmann::json j = std::any_cast<nlohmann::json>(arg);
                        append_tlv(buffer, Tag::JSON_STRING, create_value_buffer(j.dump()));
                    } catch (const std::bad_any_cast& e) {
                        Log::warn("Protocol::serialize - Failed to cast to JSON: {}", e.what());
                        throw;
                    }
                }
            }
        } catch (const std::bad_any_cast& e) {
            Log::warn("Protocol::serialize - Bad any cast: {}", e.what());
            throw;
        } catch (const std::exception& e) {
            Log::warn("Protocol::serialize - Unexpected error: {}", e.what());
            throw;
        }
        return buffer;
    }

    // --- 역직렬화 헬퍼 (Value 파싱) ---
    std::any read_value(Tag tag, const char* buffer, size_t len) {
        size_t offset = 0;
        switch(tag) {
            case Tag::UTF8_STRING:
            case Tag::EVENT_NAME: {
                uint32_t str_len = read_u32(buffer, offset);
                return std::string(buffer + offset, str_len);
            }
            case Tag::BINARY_DATA: {
                return std::vector<char>(buffer, buffer + len);
            }
            case Tag::JSON_STRING: {
                uint32_t str_len = read_u32(buffer, offset);
                // JSON 문자열 자체를 반환, 파싱은 상위 레벨에서
                return std::string(buffer + offset, str_len);
            }
            case Tag::STRING_VECTOR: {
                std::vector<std::string> vec;
                uint32_t vec_size = read_u32(buffer, offset);
                vec.reserve(vec_size);
                for(uint32_t i = 0; i < vec_size; ++i) {
                     uint32_t str_len = read_u32(buffer, offset);
                     vec.emplace_back(buffer + offset, str_len);
                     offset += str_len;
                }
                return vec;
            }
            default: return {};
        }
    }


    // --- 역직렬화 메인 함수 ---
    void deserialize(const std::vector<char>& raw_payload, std::string& out_eventName, std::vector<std::any>& out_args) {
        if (raw_payload.empty()) return;
        size_t offset = 0;
        const char* buffer = raw_payload.data();

        while (offset < raw_payload.size()) {
            Tag tag;
            memcpy(&tag, buffer + offset, 1);
            offset += 1;

            uint32_t len = read_u32(buffer, offset);

            Log::debug("Protocol::deserialize - tag: {}, len: {}, offset: {}", static_cast<int>(tag), len, offset);

            if (tag == Tag::EVENT_NAME) {
                out_eventName = std::any_cast<std::string>(read_value(Tag::EVENT_NAME, buffer + offset, len));
            } else {
                out_args.push_back(read_value(tag, buffer + offset, len));
            }
            offset += len;
        }
    }
}
