import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv

/**
 * SubtitlePopup - Dialog for subtitle track selection
 */
Dialog {
    id: root

    required property MpvObject mpvObject

    signal loadExternalClicked()

    title: "Subtitles"
    modal: true
    standardButtons: Dialog.Close

    width: 400
    height: Math.min(400, contentColumn.implicitHeight + 100)

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // "Off" option
        RadioButton {
            text: "Off"
            checked: mpvObject.currentSubtitleTrack === 0
            onClicked: mpvObject.setSubtitleTrack(0)
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Kirigami.Theme.separatorColor
            visible: mpvObject.subtitleTracks.length > 0
        }

        // Subtitle tracks
        Repeater {
            model: mpvObject.subtitleTracks

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

                    if (track.codec) {
                        label += " [" + track.codec + "]"
                    }

                    if (track.external) {
                        label += " (External)"
                    }

                    return label
                }

                checked: mpvObject.currentSubtitleTrack === modelData.id
                onClicked: mpvObject.setSubtitleTrack(modelData.id)

                Layout.fillWidth: true
            }
        }

        // Empty state
        Label {
            text: "No subtitle tracks available"
            opacity: 0.5
            visible: mpvObject.subtitleTracks.length === 0
            Layout.alignment: Qt.AlignHCenter
        }

        Item { Layout.fillHeight: true }

        // Separator before action
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Kirigami.Theme.separatorColor
        }

        // Load external button
        Button {
            text: "Load External Subtitle..."
            icon.name: "document-open"
            Layout.fillWidth: true
            onClicked: {
                root.loadExternalClicked()
                root.close()
            }
        }
    }
}
