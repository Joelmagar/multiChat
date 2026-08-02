cmake_minimum_required(VERSION 3.16)
project(ChatClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

add_executable(chat_client main.cpp chatwindow.cpp chatwindow.h)
target_link_libraries(chat_client PRIVATE Qt6::Widgets Qt6::Network)
