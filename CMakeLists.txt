cmake_minimum_required(VERSION 3.10)

project(SK2-Quiz)

set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra)

set(SOURCES
    server/server.cpp
    server/Room.cpp
    server/ClientHandler.cpp
    server/RoomHandler.cpp
    server/globals.cpp
)

set(HEADERS
    server/Room.h
    server/ClientHandler.h
    server/RoomHandler.h
    server/globals.h
    server/clientInfo.h
)

include_directories(${CMAKE_SOURCE_DIR}/server)

file(COPY ${CMAKE_SOURCE_DIR}/server/serverJSONs DESTINATION ${CMAKE_BINARY_DIR})

add_executable(SK2-Quiz-Server ${SOURCES} ${HEADERS})
