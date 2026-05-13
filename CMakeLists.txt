cmake_minimum_required(VERSION 3.24)

project(DD_SSH
    VERSION 0.1.3
    DESCRIPTION "A clean cross-platform SSH client and session manager"
    LANGUAGES CXX
)

set(DD_SSH_VERSION_STRING "dev 0.1.3.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets WebChannel)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

qt_standard_project_setup()

qt_add_executable(dd-ssh
    src/main.cpp

    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/ConnectDialog.h
    src/ui/ConnectDialog.cpp
    src/ui/BasicTerminalTab.h
    src/ui/BasicTerminalTab.cpp
    src/ui/TerminalBridge.h
    src/ui/TerminalBridge.cpp
    src/ui/WebTerminalTab.h
    src/ui/WebTerminalTab.cpp

    src/core/SessionProfile.h
    src/core/ConfigManager.h
    src/core/ConfigManager.cpp
    src/core/KnownHostsManager.h
    src/core/KnownHostsManager.cpp

    src/ssh/SshSession.h
    src/ssh/SshSession.cpp
    src/ssh/SshShellWorker.h
    src/ssh/SshShellWorker.cpp
)

target_include_directories(dd-ssh
    PRIVATE
        src
)

target_compile_definitions(dd-ssh
    PRIVATE
        DD_SSH_VERSION_STRING="${DD_SSH_VERSION_STRING}"
)

target_link_libraries(dd-ssh
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::WebEngineWidgets
        Qt6::WebChannel
        PkgConfig::LIBSSH
)

set_target_properties(dd-ssh PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
