//
// Created by 박성빈 on 25. 6. 5.
//

#include <memory>

#include "app/CollabServer.h"

int main() {
    const auto server = std::make_unique<CollabServer>();
    server->run();

    return 0;
}
