//
// Created by 박성빈 on 25. 6. 18.
//

#ifndef TYPE_H
#define TYPE_H

// === Windows (MSVC, MinGW 포함) ===
#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)

#include <winsock2.h>

using sockfd = SOCKET;
constexpr sockfd INVALID_FD = -1;

using WHAT?

// === Linux ===
#elif defined(__linux__)

#include <sys/epoll.h>

using sockfd = int;
constexpr sockfd INVALID_FD = -1;

using event = epoll_event;

// === BSD 계열 ===
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)

#include <sys/event.h>

using sockfd = int;
constexpr sockfd INVALID_FD = -1;

using event = struct kevent;

#else
#error "Unsupported platform"
#endif

#endif //TYPE_H
