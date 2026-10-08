cmake_minimum_required(VERSION 3.16)
project(ChatServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(chat_server main.cpp)
target_link_libraries(chat_server PRIVATE Threads::Threads)