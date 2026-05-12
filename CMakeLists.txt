cmake_minimum_required(VERSION 3.24)

project(DD_SSH
    VERSION 0.1.0
    DESCRIPTION "A clean cross-platform SSH client and session manager"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

qt_standard_project_setup()

qt_add_executable(dd-ssh
    src/main.cpp

    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/ConnectDialog.h
    src/ui/ConnectDialog.cpp

    src/ssh/SshSession.h
    src/ssh/SshSession.cpp
)

target_include_directories(dd-ssh
    PRIVATE
        src
)

target_link_libraries(dd-ssh
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        PkgConfig::LIBSSH
)

set_target_properties(dd-ssh PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
