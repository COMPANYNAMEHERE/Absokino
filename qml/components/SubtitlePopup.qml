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
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    signal loadExternalClicked()

    title: "Subtitles"
    modal: true
    standardButtons: Dialog.Close
    closePolicy: Popup.CloseOnEscape

    width: 400
    height: Math.min(600, contentColumn.implicitHeight + 100)

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Scrollable area for subtitle tracks
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: Math.min(400, trackList.implicitHeight + 20)

            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                id: trackList
                width: parent.width
                spacing: Kirigami.Units.smallSpacing

                // "Off" option
                RadioButton {
                    text: "Off"
                    checked: mpvObject.currentSubtitleTrack === 0
                    onClicked: mpvObject.setSubtitleTrack(0)
                    Layout.fillWidth: true
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: root.separatorColor
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
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.largeSpacing
                }
            }
        }

        // Separator before action
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: root.separatorColor
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
