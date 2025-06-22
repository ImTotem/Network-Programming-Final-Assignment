//
// Created by 박성빈 on 25. 6. 20.
//

#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <string>
#include "utils/json.hpp"
using json = nlohmann::json;

enum class FollowAction { FOLLOW, UNFOLLOW };

// OnUserFollowedPayload 구조체 정의
struct OnUserFollowedPayload {
    struct User {
        std::string socketId;
        std::string username;
    } userToFollow;
    FollowAction action;
};

// 1. User 구조체를 위한 변환 규칙
inline void to_json(json& j, const OnUserFollowedPayload::User& u) {
    j = json{
            {"socketId", u.socketId},
            {"username", u.username}
    };
}
inline void from_json(const json& j, OnUserFollowedPayload::User& u) {
    j.at("socketId").get_to(u.socketId);
    j.at("username").get_to(u.username);
}

// 2. 메인 OnUserFollowedPayload 구조체를 위한 변환 규칙
inline void to_json(json& j, const OnUserFollowedPayload& p) {
    j = json{
            {"userToFollow", p.userToFollow},
            {"action", p.action == FollowAction::FOLLOW ? "FOLLOW" : "UNFOLLOW"} // Enum을 문자열로 변환
    };
}
inline void from_json(const json& j, OnUserFollowedPayload& p) {
    j.at("userToFollow").get_to(p.userToFollow);
    std::string action_str;
    j.at("action").get_to(action_str);
    p.action = (action_str == "FOLLOW") ? FollowAction::FOLLOW : FollowAction::UNFOLLOW;
}


#endif //PAYLOAD_H
