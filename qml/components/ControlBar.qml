import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Player

/**
 * ControlBar - Bottom toolbar with playback controls
 */
ToolBar {
    id: root

    required property MpvObject mpvObject

    signal openFileClicked()
    signal libraryToggled()
    signal subtitlesClicked()
    signal audioClicked()

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: Kirigami.Theme.separatorColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Seek bar
        SeekBar {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            mpvObject: root.mpvObject
        }

        // Control buttons
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            // Library toggle button
            ToolButton {
                icon.name: "view-media-playlist"
                text: "Library"
                display: AbstractButton.IconOnly
                ToolTip.text: "Toggle Library"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                checked: PlayerController.libraryVisible
                onClicked: root.libraryToggled()
            }

            ToolSeparator {}

            // Open file button
            ToolButton {
                icon.name: "document-open"
                text: "Open"
                display: AbstractButton.IconOnly
                ToolTip.text: "Open File"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: root.openFileClicked()
            }

            ToolSeparator {}

            // Play/Pause button
            ToolButton {
                icon.name: mpvObject.paused ? "media-playback-start" : "media-playback-pause"
                text: mpvObject.paused ? "Play" : "Pause"
                display: AbstractButton.IconOnly
                ToolTip.text: mpvObject.paused ? "Play (Space)" : "Pause (Space)"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: mpvObject.togglePause()
            }

            // Stop button
            ToolButton {
                icon.name: "media-playback-stop"
                text: "Stop"
                display: AbstractButton.IconOnly
                ToolTip.text: "Stop"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: mpvObject.stop()
            }

            ToolSeparator {}

            // Time display
            Label {
                text: PlayerController.formatTime(mpvObject.position) + " / " + PlayerController.formatTime(mpvObject.duration)
                font.family: "monospace"
                Layout.minimumWidth: 120
            }

            Item { Layout.fillWidth: true }

            // Subtitle button
            ToolButton {
                icon.name: "media-view-subtitles-symbolic"
                text: "Subtitles"
                display: AbstractButton.IconOnly
                ToolTip.text: "Subtitles"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: root.subtitlesClicked()

                Rectangle {
                    visible: mpvObject.currentSubtitleTrack > 0
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 4
                    width: 8
                    height: 8
                    radius: 4
                    color: Kirigami.Theme.positiveTextColor
                }
            }

            // Audio button
            ToolButton {
                icon.name: mpvObject.muted ? "audio-volume-muted" : "audio-volume-high"
                text: "Audio"
                display: AbstractButton.IconOnly
                ToolTip.text: "Audio Settings"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: root.audioClicked()
            }

            ToolSeparator {}

            // Volume slider (compact)
            VolumeSlider {
                Layout.preferredWidth: 100
                mpvObject: root.mpvObject
            }

            ToolSeparator {}

            // Fullscreen button
            ToolButton {
                icon.name: PlayerController.isFullscreen ? "view-restore" : "view-fullscreen"
                text: "Fullscreen"
                display: AbstractButton.IconOnly
                ToolTip.text: PlayerController.isFullscreen ? "Exit Fullscreen (Esc)" : "Fullscreen (F)"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: PlayerController.isFullscreen = !PlayerController.isFullscreen
            }
        }
    }
}
