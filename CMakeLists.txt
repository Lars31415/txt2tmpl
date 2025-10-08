cmake_minimum_required(VERSION 3.5)

project(txt2tmpl VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(txt2tmpl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ../../../../../Source/Sandbox/RaspberryPiPico/tinyjson/tinyjson.hpp
        ../utils/file_utils.cpp
        ../utils/file_utils.hpp
        ../../../../../Source/Sandbox/RaspberryPiPico/utils/time_strings.cpp
        ../../../../../Source/Sandbox/RaspberryPiPico/utils/time_strings.hpp
        mainwindow.rc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET txt2tmpl APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(txt2tmpl SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(txt2tmpl
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(txt2tmpl PRIVATE
../../../../../Source/Sandbox/RaspberryPiPico/tinyjson
../utils
../../../../../Source/Sandbox/RaspberryPiPico/utils
)

target_link_libraries(txt2tmpl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.txt2tmpl)
endif()
set_target_properties(txt2tmpl PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS txt2tmpl
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(txt2tmpl)
endif()
