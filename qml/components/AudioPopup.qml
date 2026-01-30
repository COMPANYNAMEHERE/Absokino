import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv

/**
 * AudioPopup - Dialog for audio track selection and volume control
 */
Dialog {
    id: root

    required property MpvObject mpvObject

    title: "Audio"
    modal: true
    standardButtons: Dialog.Close

    width: 400
    height: Math.min(450, contentColumn.implicitHeight + 100)

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
                        to: 150
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
                        model: [0, 25, 50, 75, 100, 125, 150]

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
            color: Kirigami.Theme.separatorColor
        }

        // Audio tracks section
        Label {
            text: "Audio Tracks"
            font.weight: Font.Medium
        }

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
        }

        Item { Layout.fillHeight: true }
    }
}
