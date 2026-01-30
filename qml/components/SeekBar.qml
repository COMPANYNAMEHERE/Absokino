import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Player

/**
 * SeekBar - Timeline/seek slider with chapter markers
 */
Item {
    id: root

    required property MpvObject mpvObject
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    height: 24

    // Background track
    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 4
        radius: 2
        color: Kirigami.Theme.backgroundColor
        border.width: 1
        border.color: root.separatorColor

        // Progress fill
        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: mpvObject.duration > 0 ? parent.width * (mpvObject.position / mpvObject.duration) : 0
            radius: 2
            color: Kirigami.Theme.highlightColor
        }

        // A-B Loop markers
        Rectangle {
            visible: mpvObject.loopA >= 0 && mpvObject.duration > 0
            x: parent.width * (mpvObject.loopA / mpvObject.duration) - 2
            width: 4
            height: parent.height + 8
            anchors.verticalCenter: parent.verticalCenter
            color: Kirigami.Theme.positiveTextColor
            radius: 1

            Label {
                anchors.bottom: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                text: "A"
                font.pointSize: 8
                font.bold: true
                color: Kirigami.Theme.positiveTextColor
            }
        }

        Rectangle {
            visible: mpvObject.loopB >= 0 && mpvObject.duration > 0
            x: parent.width * (mpvObject.loopB / mpvObject.duration) - 2
            width: 4
            height: parent.height + 8
            anchors.verticalCenter: parent.verticalCenter
            color: Kirigami.Theme.negativeTextColor
            radius: 1

            Label {
                anchors.bottom: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                text: "B"
                font.pointSize: 8
                font.bold: true
                color: Kirigami.Theme.negativeTextColor
            }
        }

        // Chapter markers
        Repeater {
            model: mpvObject.chapters

            Rectangle {
                required property var modelData
                required property int index

                visible: mpvObject.duration > 0
                x: parent.width * (modelData.time / mpvObject.duration) - 1
                width: 2
                height: parent.height + 4
                anchors.verticalCenter: parent.verticalCenter
                color: Kirigami.Theme.textColor
                opacity: 0.4
            }
        }
    }

    // Interactive slider overlay
    MouseArea {
        id: seekArea
        anchors.fill: parent
        hoverEnabled: true

        property bool seeking: false

        onPressed: (mouse) => {
            seeking = true
            seek(mouse.x)
        }

        onReleased: {
            seeking = false
        }

        onPositionChanged: (mouse) => {
            if (seeking && pressed) {
                seek(mouse.x)
            }
        }

        function seek(x) {
            if (mpvObject.duration > 0) {
                let percent = Math.max(0, Math.min(100, (x / width) * 100))
                mpvObject.seekPercent(percent)
            }
        }

        // Hover preview tooltip
        ToolTip {
            id: seekTooltip
            visible: seekArea.containsMouse && mpvObject.duration > 0
            text: {
                if (mpvObject.duration > 0) {
                    let percent = seekArea.mouseX / seekArea.width
                    let time = percent * mpvObject.duration
                    return PlayerController.formatTime(time)
                }
                return ""
            }

            x: Math.max(0, Math.min(seekArea.mouseX - width / 2, seekArea.width - width))
            y: -height - 5
        }
    }

    // Seek handle
    Rectangle {
        id: handle
        visible: mpvObject.duration > 0
        x: track.width * (mpvObject.position / mpvObject.duration) - width / 2
        anchors.verticalCenter: parent.verticalCenter
        width: 14
        height: 14
        radius: 7
        color: seekArea.containsMouse || seekArea.seeking ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

        Behavior on color {
            ColorAnimation { duration: 100 }
        }
    }
}
