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
    onWidthChanged: if (!PlayerController.isFullscreen) Settings.windowSize = Qt.size(width, height)
    onHeightChanged: if (!PlayerController.isFullscreen) Settings.windowSize = Qt.size(width, height)

    // Fullscreen handling
    visibility: PlayerController.isFullscreen ? Window.FullScreen : Window.Windowed

    readonly property int maxVolume: Settings.allowVolumeBoost ? 150 : 100

    Connections {
        target: Settings
        function onAllowVolumeBoostChanged() {
            if (!Settings.allowVolumeBoost && mpv.volume > 100) {
                mpv.volume = 100
            }
        }
    }

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
        onActivated: mpv.volume = Math.min(root.maxVolume, mpv.volume + 5)
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

        MenuItem {
            text: "Subtitles..."
            onTriggered: subtitlePopup.open()
        }

        Menu {
            id: audioMenu
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
                onObjectAdded: (index, object) => audioMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => audioMenu.removeItem(object)
            }
        }

        Menu {
            id: chaptersMenu
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
                onObjectAdded: (index, object) => chaptersMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => chaptersMenu.removeItem(object)
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
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        visible: !PlayerController.isFullscreen

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

                Rectangle {
                    anchors.fill: parent
                    color: Kirigami.Theme.backgroundColor
                }

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

                // Mouse handling for windowed mode
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    z: 5
                    enabled: !PlayerController.isFullscreen

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

    // Fullscreen overlay - covers entire window
    Rectangle {
        id: fullscreenOverlay
        anchors.fill: parent
        color: "black"
        visible: PlayerController.isFullscreen
        z: 99

        // Fullscreen mouse handling
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: false

            property real lastClickTime: 0
            property real doubleClickThreshold: 300  // ms

            onClicked: (mouse) => {
                if (mouse.button === Qt.LeftButton) {
                    let currentTime = Date.now()
                    if (currentTime - lastClickTime < doubleClickThreshold) {
                        PlayerController.isFullscreen = false
                        lastClickTime = 0
                    } else {
                        lastClickTime = currentTime
                    }
                }
            }
        }
    }

    // The MpvObject - positioned explicitly to avoid reparenting issues
    MpvObject {
        id: mpv
        visible: true
        z: PlayerController.isFullscreen ? 100 : 1

        // Use explicit positioning instead of reparenting
        x: PlayerController.isFullscreen ? 0 : videoContainer.x + (libraryDrawer.visible ? libraryDrawer.width : 0)
        y: PlayerController.isFullscreen ? 0 : statusBar.height
        width: PlayerController.isFullscreen ? root.width : videoContainer.width
        height: PlayerController.isFullscreen ? root.height : videoContainer.height

        Component.onCompleted: {
            console.log("MpvObject created")
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
        z: 200

        Connections {
            target: mpv
            function onErrorOccurred(error) {
                errorBanner.text = error
                errorBanner.visible = true
            }
        }
    }
}
