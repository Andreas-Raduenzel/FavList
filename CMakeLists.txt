cmake_minimum_required(VERSION 3.16)

# 🔹 Projektname & Version
project(favlist VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 🔹 Qt6-Module, die du verwendest
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
)

qt_standard_project_setup()

# 🔹 Ressourcen (QML, Icons, etc.) aus resources.qrc
#  -> In der qrc solltest du qml/main.qml und Icons eingebunden haben.
qt_add_resources(APP_RESOURCES resources.qrc)

# 🔹 Haupt-Binary "favlist"
qt_add_executable(favlist
    src/main.cpp
    src/FavoriteBackend.cpp
    src/autostartmanager.cpp
    src/AppSettings.cpp

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

# 🔹 Standard-Installpfade (bin, share, etc.)
include(GNUInstallDirs)

# 1️⃣ Binary nach /usr/bin/favlist
install(TARGETS favlist
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# 2️⃣ .desktop-Datei nach /usr/share/applications
#    → In resources/desktop/favlist.desktop sollte stehen:
#       Exec=favlist
#       Icon=favlist
install(FILES resources/desktop/favlist.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# 3️⃣ Icon nach /usr/share/icons/hicolor/scalable/apps/favlist.svg
install(FILES resources/icons/appicon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME favlist.svg)

# (Optional) falls du QML NICHT über qrc einbindest, sondern von der Platte lädst:
# install(DIRECTORY qml/
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/favlist/qml)

# 🔹 CPack-Konfiguration für .deb
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "favlist")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andreas <andreas@example.com>")

# ✨ WICHTIG: sorgt dafür, dass dpkg-shlibdeps läuft
# und alle benötigten Bibliotheks-Abhängigkeiten (libqt6core6, libqt6gui6, ...)
# automatisch in das .deb als Depends: eingetragen werden.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# (Optional) Falls du zusätzlich sicherstellen willst, dass bestimmte QML-Module
# installiert sind, kannst du das manuell ergänzen, z. B.:
# set(CPACK_DEBIAN_PACKAGE_DEPENDS
#     "qml6-module-qtquick-controls2, qml6-module-qtquick-window, qml6-module-qtquick-layouts"
# )

include(CPack)
