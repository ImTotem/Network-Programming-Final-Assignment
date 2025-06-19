//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef SHA1_H
#define SHA1_H

#include <cstdint>
#include <iostream>
#include <string>

class SHA1 {
public:
    SHA1();
    void update(const std::string &s);
    void update(std::istream &is);
    std::string final(); // 최종 20바이트 해시를 string으로 반환

private:
    uint32_t digest[5];
    std::string buffer;
    uint64_t transforms;
};

#endif //SHA1_H
