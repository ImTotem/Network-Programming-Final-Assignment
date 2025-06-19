//
// Created by 박성빈 on 25. 6. 12.
//

#include "IOMultiplexer.h"

#include <cstring>

#include <string_view>
#include <unistd.h>
#include <sys/epoll.h>

#include "IOMultiplexingException.h"
#include "log/Log.h"

void IOMultiplexer::create() {
    iofd = epoll_create1(0);
    if (iofd == INVALID_FD) {
        close();
        Log::error<IOMultiplexingCreationException>("epoll_create1 failed");
    }
}

void IOMultiplexer::control(const Operation op, const sockfd fd) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    ev.data.fd = fd;
    if (epoll_ctl(iofd, static_cast<int>(op), fd, &ev) == INVALID_FD) {
        close();
        Log::error<IOMultiplexingControlException>("{}: fd({})", errno ? strerror(errno) : "unknown error", fd);
    }
}

std::vector<sockfd> IOMultiplexer::wait() {
    int nfds = epoll_wait(iofd, events.data(), MAX_EVENTS, -1);
    if (nfds == INVALID_FD) {
        if (errno == EINTR) return {};
        close();
        Log::error<IOMultiplexingWaitException>(errno ? strerror(errno) : "unknown error");
    }
    std::vector<sockfd> fds(nfds);
    for (int i = 0 ; i < nfds; i++) {
        fds[i] = events[i].data.fd;
    }
    return fds;
}

void IOMultiplexer::close() {
    close(iofd);
    iofd = INVALID_FD;
}

void IOMultiplexer::close(const sockfd fd) {
    if (fd == INVALID_FD) {
        return;
    }

    if (::close(fd) < 0) {
        Log::error<IOMultiplexingCloseException>(errno ? strerror(errno) : "unknown error");
    }
}

