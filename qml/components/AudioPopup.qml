import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Settings

/**
 * AudioPopup - Dialog for audio track selection and volume control
 */
Dialog {
    id: root

    required property MpvObject mpvObject
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    title: "Audio"
    modal: true
    standardButtons: Dialog.Close
    closePolicy: Popup.CloseOnEscape

    width: 400
    height: Math.min(600, contentColumn.implicitHeight + 100)
    readonly property var volumePresets: Settings.allowVolumeBoost
        ? [0, 25, 50, 75, 100, 125, 150]
        : [0, 25, 50, 75, 100]

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Volume control section
        GroupBox {
            title: "Volume"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true

                    ToolButton {
                        icon.name: mpvObject.muted ? "audio-volume-muted" : "audio-volume-high"
                        onClicked: mpvObject.muted = !mpvObject.muted
                    }

                    Slider {
                        id: volumeSlider
                        Layout.fillWidth: true
                        from: 0
                        to: Settings.allowVolumeBoost ? 150 : 100
                        value: mpvObject.volume
                        stepSize: 1

                        onMoved: mpvObject.volume = value
                    }

                    Label {
                        text: mpvObject.volume + "%"
                        font.family: "monospace"
                        Layout.minimumWidth: 45
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // Preset buttons
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Repeater {
                        model: root.volumePresets

                        Button {
                            text: modelData + "%"
                            flat: true
                            checked: mpvObject.volume === modelData
                            onClicked: mpvObject.volume = modelData
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: root.separatorColor
        }

        // Audio tracks section header
        Label {
            text: "Audio Tracks"
            font.weight: Font.Medium
        }

        // Scrollable area for audio tracks
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: Math.min(300, trackList.implicitHeight + 20)

            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                id: trackList
                width: parent.width
                spacing: Kirigami.Units.smallSpacing

                // Audio tracks
                Repeater {
                    model: mpvObject.audioTracks

                    RadioButton {
                        required property var modelData
                        required property int index

                        text: {
                            let track = modelData
                            let label = ""

                            if (track.lang && track.title) {
                                label = track.lang.toUpperCase() + " - " + track.title
                            } else if (track.lang) {
                                label = track.lang.toUpperCase()
                            } else if (track.title) {
                                label = track.title
                            } else {
                                label = "Track " + track.id
                            }

                            // Add codec info
                            let info = []
                            if (track.codec) {
                                info.push(track.codec)
                            }
                            if (track["audio-channels"]) {
                                let ch = track["audio-channels"]
                                if (ch === 2) info.push("Stereo")
                                else if (ch === 6) info.push("5.1")
                                else if (ch === 8) info.push("7.1")
                                else info.push(ch + "ch")
                            }
                            if (track["demux-samplerate"]) {
                                info.push((track["demux-samplerate"] / 1000).toFixed(1) + " kHz")
                            }

                            if (info.length > 0) {
                                label += " [" + info.join(", ") + "]"
                            }

                            if (track.default) {
                                label += " (Default)"
                            }

                            return label
                        }

                        checked: mpvObject.currentAudioTrack === modelData.id
                        onClicked: mpvObject.setAudioTrack(modelData.id)

                        Layout.fillWidth: true
                    }
                }

                // Empty state
                Label {
                    text: "No audio tracks available"
                    opacity: 0.5
                    visible: mpvObject.audioTracks.length === 0
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                }
            }
        }
    }
}
