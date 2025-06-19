//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "middleware/Middleware.h"

namespace WebSocketMiddleware {
    void handleHandshake(Context& ctx);
    void handleFrames(Context& ctx);

    std::vector<char> createFrame(const std::vector<char>& payload, uint8_t opcode = 0x2); // 0x2=Binary
}

#endif // WEBSOCKET_H