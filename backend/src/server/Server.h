//
// Created by 박성빈 on 25. 6. 12.
//

#ifndef SERVER_H
#define SERVER_H
#include <memory>

#include "event/EventEmitter.h"
#include "socket/Socket.h"
#include "socket/IOMultiplexer/IOMultiplexer.h"


class Server : public EventEmitter {
    const std::unique_ptr<Socket> socket;
    const std::unique_ptr<IOMultiplexer> io = std::make_unique<IOMultiplexer>();
    std::map<int, std::vector<char>> client_buffers;
    std::vector<char> recv_buffer;
public:
    explicit Server(unsigned short port): socket(std::make_unique<Socket>(port)), recv_buffer(4096) {}
    ~Server() override = default;

    [[noreturn]] void start();
    void stop();
    void close(sockfd fd);

    void write_raw(sockfd fd, const std::string& data);
    void write(sockfd fd, const std::vector<char>& data);
    void read(sockfd fd);

    [[nodiscard]] virtual std::vector<int> getAllFd() const {
        return {};
    }

private:
    void handleNewConnection();
    void handleStdinInput();
};



#endif //SERVER_H
