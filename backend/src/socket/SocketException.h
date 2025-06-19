//
// Created by 박성빈 on 25. 6. 18.
//

#ifndef SOCKETEXCEPTION_H
#define SOCKETEXCEPTION_H

#include <stdexcept>
#include <string>

class SocketException : public std::runtime_error {
public:
    explicit SocketException(const std::string& detail)
        : std::runtime_error("Socket Error: " + detail) {}
};

class SocketCreationException final : public SocketException {
public:
    explicit SocketCreationException(const std::string& detail)
        : SocketException("Creation failed: " + detail) {}
};

class SocketBindException final : public SocketException {
public:
    explicit SocketBindException(const std::string& detail)
        : SocketException("Bind failed: " + detail) {}
};

class SocketListenException final : public SocketException {
public:
    explicit SocketListenException(const std::string& detail)
        : SocketException("Listen failed: " + detail) {}
};

class SocketCloseException final : public SocketException {
public:
    explicit SocketCloseException(const std::string& detail)
        : SocketException("Close failed: " + detail) {}
};

class SocketOptionException final : public SocketException {
public:
    explicit SocketOptionException(const std::string& detail)
        : SocketException("Socket option failed: " + detail) {}
};

#endif //SOCKETEXCEPTION_H
