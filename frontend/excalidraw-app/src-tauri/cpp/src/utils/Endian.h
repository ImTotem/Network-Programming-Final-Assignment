//
// Created by 박성빈 on 25. 6. 19.
//

#ifndef ENDIAN_H
#define ENDIAN_H

#include <cstdint>

// 플랫폼별 헤더 include
#if defined(_WIN32)
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

// 64비트 변환을 위한 크로스플랫폼 처리
#if defined(__APPLE__) || defined(__FreeBSD__)
    #include <libkern/OSByteOrder.h>
    #define htobe64(x) OSSwapHostToBigInt64(x)
    #define be64toh(x) OSSwapBigToHostInt64(x)
#elif defined(_WIN32)
    // 윈도우는 기본적으로 리틀 엔디안
    #define htobe64(x) _byteswap_uint64(x)
    #define be64toh(x) _byteswap_uint64(x)
#else // Linux 등
    #include <endian.h>
#endif


namespace Endian {
    inline uint16_t hostToNetwork16(uint16_t hostshort) {
        return htons(hostshort);
    }
    inline uint16_t networkToHost16(uint16_t netshort) {
        return ntohs(netshort);
    }
    inline uint32_t hostToNetwork32(uint32_t hostlong) {
        return htonl(hostlong);
    }
    inline uint32_t networkToHost32(uint32_t netlong) {
        return ntohl(netlong);
    }
    inline uint64_t hostToNetwork64(uint64_t hostlonglong) {
        return htobe64(hostlonglong);
    }
    inline uint64_t networkToHost64(uint64_t netlonglong) {
        return be64toh(netlonglong);
    }
}

#endif //ENDIAN_H

