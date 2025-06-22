cmake_minimum_required(VERSION 3.28.3)
project(collab_client C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


set(COLLAB_CXX_SOURCES
        src/CollabClient.h
        src/CollabClient.cpp
        src/log/Log.h
        src/log/Log.cpp
        src/utils/json.hpp
        src/CollabClientCAPI.h
        src/CollabClientCAPI.cpp
)

set_source_files_properties(${COLLAB_C_SOURCES} PROPERTIES LANGUAGE C)

add_library(
        collab_lib STATIC
        ${COLLAB_C_SOURCES}
        ${COLLAB_CXX_SOURCES}
)

target_include_directories(collab_lib PRIVATE src)
