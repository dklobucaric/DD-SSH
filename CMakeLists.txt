cmake_minimum_required(VERSION 3.24)

project(DD_SSH
    VERSION 0.1.7
    DESCRIPTION "A clean cross-platform SSH client and session manager"
    LANGUAGES CXX
)

set(DD_SSH_VERSION_STRING "dev 0.1.7.7")
set(DD_SSH_CODENAME_STRING "Andromeda")
set(DD_SSH_MILESTONE_STRING "Single-file SFTP upload foundation")

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
    src/ui/SettingsDialog.h
    src/ui/SettingsDialog.cpp
    src/ui/SftpBrowserTab.h
    src/ui/SftpBrowserTab.cpp

    src/core/SessionProfile.h
    src/core/ConfigManager.h
    src/core/ConfigManager.cpp
    src/core/KnownHostsManager.h
    src/core/KnownHostsManager.cpp
    src/core/AppLogger.h
    src/core/AppLogger.cpp

    src/ssh/SshCompatibility.h
    src/ssh/SshCompatibility.cpp
    src/ssh/SshSession.h
    src/ssh/SshSession.cpp
    src/ssh/SshShellWorker.h
    src/ssh/SshShellWorker.cpp

    src/sftp/SftpProbe.h
    src/sftp/SftpProbe.cpp
)

qt_add_resources(dd-ssh "xterm_assets"
    PREFIX "/xterm"
    BASE "resources/xterm"
    FILES
        resources/xterm/xterm.js
        resources/xterm/xterm.css
        resources/xterm/addon-fit.js
        resources/xterm/LICENSE.xterm.txt
        resources/xterm/LICENSE.addon-fit.txt
        resources/xterm/THIRD_PARTY_NOTICES.md
)

qt_add_resources(dd-ssh "icon_assets"
    PREFIX "/icons"
    BASE "resources/icons"
    FILES
        resources/icons/dd-ssh.png
        resources/icons/dd-ssh-16.png
        resources/icons/dd-ssh-24.png
        resources/icons/dd-ssh-32.png
        resources/icons/dd-ssh-48.png
        resources/icons/dd-ssh-64.png
        resources/icons/dd-ssh-128.png
        resources/icons/dd-ssh-256.png
        resources/icons/dd-ssh-512.png
        resources/icons/dd-ssh-1024.png
)

if(WIN32)
    target_sources(dd-ssh PRIVATE
        resources/windows/dd-ssh.rc
    )
endif()

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE dd-ssh.icns)
    set_source_files_properties(resources/macos/dd-ssh.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(dd-ssh PRIVATE
        resources/macos/dd-ssh.icns
    )
endif()

target_include_directories(dd-ssh
    PRIVATE
        src
)

target_compile_definitions(dd-ssh
    PRIVATE
        DD_SSH_VERSION_STRING="${DD_SSH_VERSION_STRING}"
        DD_SSH_CODENAME_STRING="${DD_SSH_CODENAME_STRING}"
        DD_SSH_MILESTONE_STRING="${DD_SSH_MILESTONE_STRING}"
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
    MACOSX_BUNDLE_ICON_FILE dd-ssh.icns
)


include(GNUInstallDirs)

install(TARGETS dd-ssh
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

install(FILES resources/linux/dd-ssh.desktop.in
    RENAME dd-ssh.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

foreach(icon_size 16 24 32 48 64 128 256 512 1024)
    install(FILES resources/icons/dd-ssh-${icon_size}.png
        RENAME dd-ssh.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${icon_size}x${icon_size}/apps
    )
endforeach()

install(FILES
        README.md
        LICENSE
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/dd-ssh
)

install(DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/dd-ssh/docs
    FILES_MATCHING
        PATTERN "*.md"
        PATTERN "*.png"
)
