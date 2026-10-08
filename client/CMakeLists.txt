cmake_minimum_required(VERSION 3.16)

project(chat_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Network
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

qt_add_executable(chat_client
    main.cpp
    chatwindow.cpp
    chatwindow.h
    cryptohelper.cpp
    cryptohelper.h
)

target_include_directories(chat_client PRIVATE
    ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(chat_client PRIVATE
    Qt6::Widgets
    Qt6::Network
    ${SODIUM_LIBRARIES}
)