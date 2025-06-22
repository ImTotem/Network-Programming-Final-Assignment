//
// Created by 박성빈 on 25. 6. 21.
//


#ifndef ENDIAN_H
#define ENDIAN_H

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
    using ::htons;
    using ::ntohs;
    using ::htonl;
    using ::ntohl;
}

#endif //ENDIAN_H

