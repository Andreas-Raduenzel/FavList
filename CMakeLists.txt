cmake_minimum_required(VERSION 3.16)
project(favoriten_v32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2)

qt_standard_project_setup()

qt_add_resources(APP_RESOURCES resources.qrc)

qt_add_executable(favoriten
    src/main.cpp
    src/FavoriteBackend.cpp

    ${APP_RESOURCES}
)

target_include_directories(favoriten PRIVATE src)

target_link_libraries(favoriten PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

install(TARGETS favoriten DESTINATION bin)
