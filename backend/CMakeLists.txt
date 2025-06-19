cmake_minimum_required(VERSION 3.28.3)
project(collab_server C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

set(COLLAB_C_SOURCES
        src/env/dotenv.c
)

set(COLLAB_CXX_SOURCES
        src/env/dotenv.h
        src/log/Log.h
        src/log/Log.cpp
        src/socket/SocketType.h
        src/socket/SocketException.h
        src/socket/Socket.h
        src/socket/IOMultiplexer/IOMultiplexer.h
        src/server/Server.h
        src/server/Server.cpp
        src/event/EventEmitter.h
        src/app/CollabServer.h
        src/app/CollabServer.cpp
        src/room/RoomServer.cpp
        src/room/RoomServer.h
        src/room/client/IdGenerator.h
        src/middleware/Middleware.h
        src/room/client/Client.cpp
        src/room/client/Client.h
        src/middleware/protocol/Protocol.h
        src/middleware/protocol/Protocol.cpp
        src/middleware/websocket/WebSocket.h
        src/middleware/websocket/WebSocket.cpp
        src/utils/Endian.h
        src/utils/sha1.h
        src/utils/base64.h
        src/utils/sha1.cpp
        src/utils/base64.cpp
        src/middleware/payload/Payload.h
)

if(WIN32)
    # Windows용 (MSVC, MinGW 등)
    message(STATUS "Platform: Windows")
    list(APPEND COLLAB_CXX_SOURCES src/socket/Socket_win.cpp)
    list(APPEND COLLAB_CXX_SOURCES src/socket/IOMultiplexer/IOMultiplexer_win.cpp)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux용
    message(STATUS "Platform: Linux")
    list(APPEND COLLAB_CXX_SOURCES src/socket/Socket_posix.cpp)
    list(APPEND COLLAB_CXX_SOURCES src/socket/IOMultiplexer/IOMultiplexer_linux.cpp)
elseif(APPLE OR CMAKE_SYSTEM_NAME MATCHES "BSD")
    # BSD 계열 (macOS, FreeBSD, OpenBSD, NetBSD 등)
    message(STATUS "Platform: BSD (${CMAKE_SYSTEM_NAME})")
    list(APPEND COLLAB_CXX_SOURCES src/socket/Socket_posix.cpp)
    list(APPEND COLLAB_CXX_SOURCES src/socket/IOMultiplexer/IOMultiplexer_bsd.cpp)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

add_library(
        collab_lib SHARED
        ${COLLAB_C_SOURCES}
        ${COLLAB_CXX_SOURCES}
)

set_source_files_properties(${COLLAB_C_SOURCES} PROPERTIES LANGUAGE C)

add_executable(
        collab_server
        src/main.cpp
)

target_include_directories(collab_lib PRIVATE src)
target_include_directories(collab_server PRIVATE src)
target_link_libraries(collab_lib PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(collab_server collab_lib)

if(WIN32)
    target_link_libraries(collab_lib PRIVATE ws2_32)
endif()
