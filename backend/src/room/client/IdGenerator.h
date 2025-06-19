//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <string>
#include <random>
#include <sstream>

namespace IdGenerator {
    inline std::string generate() {
        // 간단한 16진수 랜덤 문자열 생성 (24자)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 15);

        std::stringstream ss;
        for (int i = 0; i < 24; ++i) {
            ss << std::hex << distrib(gen);
        }
        return ss.str();
    }
}

#endif //IDGENERATOR_H
