cmake_minimum_required(VERSION 3.20)
project(TorqueClient)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)   # ADD THIS - runs the MOC compiler
set(CMAKE_AUTOUIC ON)   # ADD THIS - for .ui files
set(CMAKE_AUTORCC ON)   # ADD THIS - for .qrc resource files

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

add_executable(TorqueClient
        Customer/main.cpp
        MainWindows/MainWindow.cpp
        MainWindows/MainWindow.h
        MainWindows/LandingPage.cpp
        MainWindows/LandingPage.h
        MainWindows/SignUpPage.cpp
        MainWindows/SignUpPage.h
        MainWindows/LoginPage.cpp
        MainWindows/LoginPage.h
        MainWindows/PasswordResetPage.cpp
        MainWindows/PasswordResetPage.h
        MainWindows/HomePage.cpp
        MainWindows/HomePage.h
        Customer/AppointmnetDetailPage.cpp
        Customer/AppointmnetDetailPage.h
        Customer/BookAppointmentsPage.cpp
        Customer/BookAppointmentsPage.h
        Customer/FindAppointmentsPage.cpp
        Customer/FindAppointmentsPage.h
        Customer/JobTrackingPage.cpp
        Customer/JobTrackingPage.h
        Customer/LeaveReviewPage.cpp
        Customer/LeaveReviewPage.h
        Customer/MechanicProfilePage.cpp
        Customer/MechanicProfilePage.h
        Customer/MyAppointmentsPage.cpp
        Customer/MyAppointmentsPage.h
        Customer/MyReviews.cpp
        Customer/MyReviews.h
        Customer/MyVehiclesPage.cpp
        Customer/MyVehiclesPage.h
        Mechanic/JobDetailPage.cpp
        Mechanic/JobDetailPage.h
        Mechanic/MechanicDashboardPage.cpp
        Mechanic/MechanicDashboardPage.h
        Mechanic/MyAppointmentsPage.h
        Mechanic/MyAppointmentsPage.cpp
        Mechanic/MyJobsPage.cpp
        Mechanic/MyJobsPage.h

)

target_link_libraries(TorqueClient
        PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
)