cmake_minimum_required(VERSION 3.20)
project(TorqueClient)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)   # ADD THIS - runs the MOC compiler
set(CMAKE_AUTOUIC ON)   # ADD THIS - for .ui files
set(CMAKE_AUTORCC ON)   # ADD THIS - for .qrc resource files

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

add_executable(TorqueClient
        main.cpp
        MainWindow.cpp
        MainWindow.h
        LandingPage.cpp
        LandingPage.h
        SignUpPage.cpp
        SignUpPage.h
        LoginPage.cpp
        LoginPage.h
        PasswordResetPage.cpp
        PasswordResetPage.h
        HomePage.cpp
        HomePage.h
)

target_link_libraries(TorqueClient
        PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
)