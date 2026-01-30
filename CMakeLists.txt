cmake_minimum_required(VERSION 3.21)
project(Absokino VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Quick
    QuickControls2
    Widgets
    DBus
)

# Find Kirigami
find_package(KF6Kirigami REQUIRED)
find_package(KF6Config REQUIRED)

# Find libmpv
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)

# Sources
set(SOURCES
    src/main.cpp
    src/mpvobject.cpp
    src/mpvrenderer.cpp
    src/playercontroller.cpp
    src/settingsmanager.cpp
    src/hdrdiagnostics.cpp
    src/recentfilesmodel.cpp
    src/trackmodel.cpp
    src/chaptermodel.cpp
)

set(HEADERS
    src/mpvobject.h
    src/mpvrenderer.h
    src/playercontroller.h
    src/settingsmanager.h
    src/hdrdiagnostics.h
    src/recentfilesmodel.h
    src/trackmodel.h
    src/chaptermodel.h
)

# QML files
set(QML_FILES
    qml/Main.qml
    qml/components/VideoArea.qml
    qml/components/ControlBar.qml
    qml/components/StatusBar.qml
    qml/components/LibraryDrawer.qml
    qml/components/SubtitlePopup.qml
    qml/components/AudioPopup.qml
    qml/components/ChaptersMenu.qml
    qml/components/AdvancedMenu.qml
    qml/components/VolumeSlider.qml
    qml/components/SeekBar.qml
    qml/dialogs/SettingsDialog.qml
    qml/dialogs/DiagnosticsDialog.qml
)

qt_add_executable(absokino
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(absokino
    URI Absokino
    VERSION 1.0
    QML_FILES ${QML_FILES}
    RESOURCE_PREFIX /
)

target_include_directories(absokino PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(absokino PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::DBus
    KF6::Kirigami
    KF6::ConfigCore
    PkgConfig::MPV
)

# Install
install(TARGETS absokino
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
