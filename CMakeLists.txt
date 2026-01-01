cmake_minimum_required(VERSION 3.16)

# 🔹 Projektdefinition
project(
    FavList
    VERSION 0.1.0
    LANGUAGES CXX
)

# 🔹 CMake / Qt Automatik
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 🔹 C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 🔹 Qt 6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
)

qt_standard_project_setup()

# 🔹 Ressourcen
qt_add_resources(APP_RESOURCES resources.qrc)

# 🔹 Executable / Binary
qt_add_executable(favlist
    src/main.cpp
    src/FavoriteBackend.cpp
    src/autostartmanager.cpp
    ${APP_RESOURCES}
)

target_include_directories(favlist PRIVATE src)

target_link_libraries(favlist PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# 🔹 Installationspfade
include(GNUInstallDirs)

# 1. Binary → /usr/bin/favlist
install(TARGETS favlist
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# 2. QML → /usr/share/favlist/qml
install(DIRECTORY qml/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/favlist/qml)

# 3. Desktop-Datei
install(FILES resources/desktop/favlist.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# 4. Icon
install(FILES resources/icons/appicon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME favlist.svg)

# 🔹 CPack / DEB-Paket
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "favlist")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Crumbtech")
set(CPACK_PACKAGE_CONTACT "info@crumbtech.de")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreas Radünzel <info@crumbtech.de>")

# optional, aber nett:
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)


# Automatisch Library-Abhängigkeiten ermitteln (Qt6-Libs etc.)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# QML-Module (werden NICHT von shlibdeps erkannt!)
set(CPACK_DEBIAN_PACKAGE_DEPENDS
  "qml6-module-qtquick, \
   qml6-module-qtquick-controls, \
   qml6-module-qtquick-layouts, \
   qml6-module-qtqml-models, \
   qml6-module-qtqml-workerscript, \
   qml6-module-qtquick-window, \
   qml6-module-qtquick-templates"
)

include(CPack)

