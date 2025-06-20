cmake_minimum_required(VERSION 3.28.3)
project(collab_client C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

set(COLLAB_C_SOURCES

)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

set(COLLAB_CXX_SOURCES
        src/CollabClient.h
        src/CollabClient.cpp
        src/log/Log.h
        src/log/Log.cpp
        src/payload/Payload.h
        src/protocol/Protocol.h
        src/protocol/Protocol.cpp
        src/utils/base64.h
        src/utils/base64.cpp
        src/utils/sha1.h
        src/utils/sha1.cpp
        src/utils/Endian.h
        src/CollabClientCAPI.h
        src/CollabClientCAPI.cpp
)

set_source_files_properties(${COLLAB_C_SOURCES} PROPERTIES LANGUAGE C)

add_library(
        collab_lib STATIC
        ${COLLAB_C_SOURCES}
        ${COLLAB_CXX_SOURCES}
)

add_executable(
        collab_client
        src/main.cpp
)

target_include_directories(collab_lib PRIVATE src)
target_include_directories(collab_client PRIVATE src)
target_link_libraries(collab_lib PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(collab_client collab_lib)
