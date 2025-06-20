//
// Created by 박성빈 on 25. 6. 11.
//

#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <memory>

class RoomServer;

class CollabServer {
public:
    CollabServer();
    ~CollabServer();

    // 애플리케이션을 시작합니다.
    void run();

private:
    // RoomServer는 이제 CollabServer의 멤버로서 생명주기를 함께합니다.
    std::unique_ptr<RoomServer> io;

    // 이벤트 핸들러들을 등록하는 private 메서드
    void setupEventHandlers();
};



#endif //MAINSERVER_H
