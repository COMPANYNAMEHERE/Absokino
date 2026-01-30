import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv

/**
 * VolumeSlider - Compact volume control for the control bar
 */
RowLayout {
    id: root

    required property MpvObject mpvObject

    spacing: 4

    Slider {
        id: slider
        Layout.fillWidth: true

        from: 0
        to: 150
        value: mpvObject.volume
        stepSize: 1

        onMoved: mpvObject.volume = value

        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: slider.availableWidth
            height: 4
            radius: 2
            color: Kirigami.Theme.backgroundColor
            border.width: 1
            border.color: Kirigami.Theme.separatorColor

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                radius: 2
                color: mpvObject.muted ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.highlightColor
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 12
            height: 12
            radius: 6
            color: slider.pressed ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

            Behavior on color {
                ColorAnimation { duration: 100 }
            }
        }
    }

    Label {
        text: mpvObject.volume
        font.family: "monospace"
        font.pointSize: Kirigami.Theme.smallFont.pointSize
        Layout.minimumWidth: 25
        horizontalAlignment: Text.AlignRight
        opacity: mpvObject.muted ? 0.4 : 0.8
    }
}
