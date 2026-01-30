import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt.labs.platform as Platform
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Player
import Absokino.Settings
import Absokino.Models

import "components"
import "dialogs"

Kirigami.ApplicationWindow {
    id: root

    title: mpv.mediaTitle || mpv.filename || "Absokino"
    width: Settings.windowSize.width
    height: Settings.windowSize.height
    visible: true

    // Track window state changes
    onWidthChanged: if (!PlayerController.isFullscreen) Settings.setWindowSize(Qt.size(width, height))
    onHeightChanged: if (!PlayerController.isFullscreen) Settings.setWindowSize(Qt.size(width, height))

    // Fullscreen handling
    visibility: PlayerController.isFullscreen ? Window.FullScreen : Window.Windowed

    // Global keyboard shortcuts
    Shortcut {
        sequence: "Space"
        onActivated: mpv.togglePause()
    }

    Shortcut {
        sequence: "Left"
        onActivated: mpv.seek(-5)
    }

    Shortcut {
        sequence: "Right"
        onActivated: mpv.seek(5)
    }

    Shortcut {
        sequence: "Shift+Left"
        onActivated: mpv.seek(-30)
    }

    Shortcut {
        sequence: "Shift+Right"
        onActivated: mpv.seek(30)
    }

    Shortcut {
        sequence: "Up"
        onActivated: mpv.volume = Math.min(150, mpv.volume + 5)
    }

    Shortcut {
        sequence: "Down"
        onActivated: mpv.volume = Math.max(0, mpv.volume - 5)
    }

    Shortcut {
        sequence: "F"
        onActivated: PlayerController.isFullscreen = !PlayerController.isFullscreen
    }

    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (PlayerController.isFullscreen) {
                PlayerController.isFullscreen = false
            }
        }
    }

    Shortcut {
        sequence: ","
        onActivated: mpv.frameBackStep()
    }

    Shortcut {
        sequence: "."
        onActivated: mpv.frameStep()
    }

    Shortcut {
        sequence: "["
        onActivated: mpv.speed = Math.max(0.25, mpv.speed - 0.25)
    }

    Shortcut {
        sequence: "]"
        onActivated: mpv.speed = Math.min(4.0, mpv.speed + 0.25)
    }

    Shortcut {
        sequence: "M"
        onActivated: mpv.muted = !mpv.muted
    }

    // File dialog
    FileDialog {
        id: fileDialog
        title: "Open Video File"
        nameFilters: [
            "Video files (*.mp4 *.mkv *.avi *.mov *.webm *.m4v *.wmv *.flv *.ts *.m2ts)",
            "All files (*)"
        ]
        onAccepted: {
            PlayerController.openFileUrl(selectedFile)
        }
    }

    // Subtitle file dialog
    FileDialog {
        id: subtitleDialog
        title: "Load Subtitle File"
        nameFilters: [
            "Subtitle files (*.srt *.ass *.ssa *.sub *.vtt *.sup *.idx)",
            "All files (*)"
        ]
        onAccepted: {
            mpv.loadSubtitleFile(selectedFile.toString().replace("file://", ""))
        }
    }

    // Handle file open request
    Connections {
        target: PlayerController
        function onRequestOpenFile() {
            fileDialog.open()
        }
    }

    // Handle initial file
    Component.onCompleted: {
        if (PlayerController.initialFile.length > 0) {
            mpv.loadFile(PlayerController.initialFile)
            RecentFiles.addFile(PlayerController.initialFile)
        }
    }

    // Drop area for drag-and-drop
    DropArea {
        anchors.fill: parent
        keys: ["text/uri-list"]

        onDropped: (drop) => {
            if (drop.hasUrls && drop.urls.length > 0) {
                let url = drop.urls[0]
                let path = url.toString().replace("file://", "")
                mpv.loadFile(path)
                RecentFiles.addFile(path)
            }
        }
    }

    // Context menu
    Menu {
        id: contextMenu

        Menu {
            title: "Subtitles"

            MenuItem {
                text: "Off"
                checkable: true
                checked: mpv.currentSubtitleTrack === 0
                onTriggered: mpv.setSubtitleTrack(0)
            }

            MenuSeparator {}

            Instantiator {
                model: mpv.subtitleTracks
                MenuItem {
                    text: {
                        let track = mpv.subtitleTracks[index]
                        let label = track.title || track.lang || ("Track " + track.id)
                        if (track.lang && track.title) {
                            label = track.lang.toUpperCase() + " - " + track.title
                        } else if (track.lang) {
                            label = track.lang.toUpperCase()
                        }
                        return label
                    }
                    checkable: true
                    checked: mpv.currentSubtitleTrack === mpv.subtitleTracks[index].id
                    onTriggered: mpv.setSubtitleTrack(mpv.subtitleTracks[index].id)
                }
                onObjectAdded: (index, object) => contextMenu.children[0].insertItem(index + 2, object)
                onObjectRemoved: (index, object) => contextMenu.children[0].removeItem(object)
            }

            MenuSeparator {}

            MenuItem {
                text: "Load External..."
                onTriggered: subtitleDialog.open()
            }
        }

        Menu {
            title: "Audio"

            Instantiator {
                model: mpv.audioTracks
                MenuItem {
                    text: {
                        let track = mpv.audioTracks[index]
                        let label = track.title || track.lang || ("Track " + track.id)
                        if (track.lang && track.title) {
                            label = track.lang.toUpperCase() + " - " + track.title
                        } else if (track.lang) {
                            label = track.lang.toUpperCase()
                        }
                        if (track.codec) {
                            label += " [" + track.codec + "]"
                        }
                        return label
                    }
                    checkable: true
                    checked: mpv.currentAudioTrack === mpv.audioTracks[index].id
                    onTriggered: mpv.setAudioTrack(mpv.audioTracks[index].id)
                }
                onObjectAdded: (index, object) => contextMenu.children[1].insertItem(index, object)
                onObjectRemoved: (index, object) => contextMenu.children[1].removeItem(object)
            }
        }

        Menu {
            title: "Chapters"
            enabled: mpv.chapters.length > 0

            Instantiator {
                model: mpv.chapters
                MenuItem {
                    text: {
                        let chapter = mpv.chapters[index]
                        let title = chapter.title || ("Chapter " + (index + 1))
                        let time = PlayerController.formatTime(chapter.time)
                        return time + " - " + title
                    }
                    checkable: true
                    checked: mpv.currentChapter === index
                    onTriggered: mpv.setChapter(index)
                }
                onObjectAdded: (index, object) => contextMenu.children[2].insertItem(index, object)
                onObjectRemoved: (index, object) => contextMenu.children[2].removeItem(object)
            }
        }

        MenuSeparator {}

        Menu {
            title: "Speed"

            MenuItem { text: "0.25x"; checkable: true; checked: Math.abs(mpv.speed - 0.25) < 0.01; onTriggered: mpv.speed = 0.25 }
            MenuItem { text: "0.5x"; checkable: true; checked: Math.abs(mpv.speed - 0.5) < 0.01; onTriggered: mpv.speed = 0.5 }
            MenuItem { text: "0.75x"; checkable: true; checked: Math.abs(mpv.speed - 0.75) < 0.01; onTriggered: mpv.speed = 0.75 }
            MenuItem { text: "1.0x (Normal)"; checkable: true; checked: Math.abs(mpv.speed - 1.0) < 0.01; onTriggered: mpv.speed = 1.0 }
            MenuItem { text: "1.25x"; checkable: true; checked: Math.abs(mpv.speed - 1.25) < 0.01; onTriggered: mpv.speed = 1.25 }
            MenuItem { text: "1.5x"; checkable: true; checked: Math.abs(mpv.speed - 1.5) < 0.01; onTriggered: mpv.speed = 1.5 }
            MenuItem { text: "1.75x"; checkable: true; checked: Math.abs(mpv.speed - 1.75) < 0.01; onTriggered: mpv.speed = 1.75 }
            MenuItem { text: "2.0x"; checkable: true; checked: Math.abs(mpv.speed - 2.0) < 0.01; onTriggered: mpv.speed = 2.0 }
        }

        Menu {
            title: "A-B Loop"

            MenuItem {
                text: mpv.loopA >= 0 ? "Set A (current: " + PlayerController.formatTime(mpv.loopA) + ")" : "Set A"
                onTriggered: mpv.setLoopA()
            }
            MenuItem {
                text: mpv.loopB >= 0 ? "Set B (current: " + PlayerController.formatTime(mpv.loopB) + ")" : "Set B"
                onTriggered: mpv.setLoopB()
            }
            MenuItem {
                text: "Clear Loop"
                enabled: mpv.loopA >= 0 || mpv.loopB >= 0
                onTriggered: mpv.clearLoop()
            }
        }

        MenuSeparator {}

        MenuItem {
            text: "Frame Step Forward"
            onTriggered: mpv.frameStep()
        }

        MenuItem {
            text: "Frame Step Backward"
            onTriggered: mpv.frameBackStep()
        }

        MenuSeparator {}

        MenuItem {
            text: "Settings..."
            onTriggered: settingsDialog.open()
        }
    }

    // Diagnostics dialog (declare before settings so it can be referenced)
    DiagnosticsDialog {
        id: diagnosticsDialog
        mpvObject: mpv
    }

    // Settings dialog
    SettingsDialog {
        id: settingsDialog
        mpvObject: mpv
        diagnosticsDialog: diagnosticsDialog
    }

    // Main content - windowed mode
    Item {
        anchors.fill: parent
        visible: !PlayerController.isFullscreen

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // Status bar (top)
            StatusBar {
                id: statusBar
                Layout.fillWidth: true
                mpvObject: mpv
                onSettingsClicked: settingsDialog.open()
                onDiagnosticsClicked: diagnosticsDialog.open()
            }

            // Main content area with Library drawer
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                // Library drawer
                LibraryDrawer {
                    id: libraryDrawer
                    Layout.fillHeight: true
                    visible: PlayerController.libraryVisible
                    onFileSelected: (path) => {
                        mpv.loadFile(path)
                        RecentFiles.addFile(path)
                    }
                }

                // Video container for windowed mode
                Item {
                    id: videoContainer
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // "No video" placeholder
                    Item {
                        anchors.centerIn: parent
                        visible: !mpv.playing && mpv.duration <= 0
                        opacity: 0.5
                        z: 10

                        Column {
                            anchors.centerIn: parent
                            spacing: Kirigami.Units.largeSpacing

                            Kirigami.Icon {
                                source: "video-symbolic"
                                width: Kirigami.Units.iconSizes.enormous
                                height: width
                                anchors.horizontalCenter: parent.horizontalCenter
                                opacity: 0.3
                            }

                            Label {
                                text: "Drop a video file here or click Open"
                                color: Kirigami.Theme.disabledTextColor
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    // Mouse handling for windowed mode (behind the video)
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        z: 5

                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton) {
                                contextMenu.popup()
                            }
                        }

                        onDoubleClicked: {
                            PlayerController.isFullscreen = true
                        }
                    }
                }
            }

            // Control bar (bottom)
            ControlBar {
                id: controlBar
                Layout.fillWidth: true
                mpvObject: mpv

                onOpenFileClicked: fileDialog.open()
                onLibraryToggled: PlayerController.libraryVisible = !PlayerController.libraryVisible
                onSubtitlesClicked: subtitlePopup.open()
                onAudioClicked: audioPopup.open()
            }
        }
    }

    // The MpvObject instance - always stays in the same place
    Item {
        id: videoRenderContainer
        anchors.fill: parent
        z: PlayerController.isFullscreen ? 100 : 0  // Bring to front in fullscreen

        Rectangle {
            anchors.fill: parent
            color: PlayerController.isFullscreen ? "black" : Kirigami.Theme.backgroundColor
            visible: PlayerController.isFullscreen
        }

        MpvObject {
            id: mpv
            anchors.fill: parent
            visible: true

            Component.onCompleted: {
                console.log("MpvObject created, size:", width, "x", height)
            }

            onWidthChanged: console.log("MpvObject width changed:", width)
            onHeightChanged: console.log("MpvObject height changed:", height)
        }

        // Fullscreen mouse handling overlay
        MouseArea {
            anchors.fill: parent
            visible: PlayerController.isFullscreen
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: false

            property real lastClickTime: 0
            property real doubleClickThreshold: 300  // ms

            onClicked: (mouse) => {
                // In fullscreen: single clicks do NOTHING (as specified)
                // But we need to detect double-click manually
                if (mouse.button === Qt.LeftButton) {
                    let currentTime = Date.now()
                    if (currentTime - lastClickTime < doubleClickThreshold) {
                        PlayerController.isFullscreen = false
                        lastClickTime = 0
                    } else {
                        lastClickTime = currentTime
                    }
                }
                // Right clicks also do nothing in fullscreen
            }
        }
    }

    // Popups
    SubtitlePopup {
        id: subtitlePopup
        mpvObject: mpv
        onLoadExternalClicked: subtitleDialog.open()
    }

    AudioPopup {
        id: audioPopup
        mpvObject: mpv
    }

    // Error banner
    Kirigami.InlineMessage {
        id: errorBanner
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: Kirigami.Units.smallSpacing
        visible: false
        type: Kirigami.MessageType.Error
        showCloseButton: true
        z: 100

        Connections {
            target: mpv
            function onErrorOccurred(error) {
                errorBanner.text = error
                errorBanner.visible = true
            }
        }
    }
}
