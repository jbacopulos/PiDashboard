cmake_minimum_required(VERSION 3.16)

project(PiDashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Xml)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    resources.qrc
    XmlReader.h
    XmlReader.cpp
    ImageDownloader.h
    ImageDownloader.cpp
)

if(ANDROID)
    add_library(PiDashboard SHARED
        ${PROJECT_SOURCES}
    )
    # Define properties for Android with Qt 5
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    add_executable(PiDashboard
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(PiDashboard PRIVATE Qt5::Widgets Qt5::Network Qt5::Xml)

# macOS/iOS-specific bundle properties
set_target_properties(PiDashboard PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PiDashboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
