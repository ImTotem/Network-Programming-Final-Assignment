//
// Created by 박성빈 on 25. 6. 18.
//

#ifndef IOMultiplexingException_H
#define IOMultiplexingException_H

#include <stdexcept>
#include <string>

class IOMultiplexingException : public std::runtime_error {
public:
    explicit IOMultiplexingException(const std::string& detail)
        : std::runtime_error("IOMultiplexing Error: " + detail) {}
};

class IOMultiplexingCreationException final : public IOMultiplexingException {
public:
    explicit IOMultiplexingCreationException(const std::string& detail)
        : IOMultiplexingException("Creation failed: " + detail) {}
};

class IOMultiplexingControlException final : public IOMultiplexingException {
public:
    explicit IOMultiplexingControlException(const std::string& detail)
        : IOMultiplexingException("Control failed: " + detail) {}
};

class IOMultiplexingWaitException final : public IOMultiplexingException {
public:
    explicit IOMultiplexingWaitException(const std::string& detail)
        : IOMultiplexingException("Wait failed: " + detail) {}
};

class IOMultiplexingCloseException final : public IOMultiplexingException {
public:
    explicit IOMultiplexingCloseException(const std::string& detail)
        : IOMultiplexingException("Close failed: " + detail) {}
};

#endif //IOMultiplexingException_H
