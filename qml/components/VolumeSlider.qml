import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Settings

/**
 * VolumeSlider - Compact volume control for the control bar
 */
RowLayout {
    id: root

    required property MpvObject mpvObject
    // Scale the control bar volume slider without relying on layout defaults.
    property real widthScale: 0.3
    readonly property real baseWidth: slider.implicitWidth > 0 ? slider.implicitWidth : Kirigami.Units.gridUnit * 8
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    spacing: 4
    implicitWidth: Math.max(1, Math.round(baseWidth * widthScale))
    implicitHeight: slider.implicitHeight

    Slider {
        id: slider
        Layout.fillWidth: true

        from: 0
        to: Settings.allowVolumeBoost ? 150 : 100
        value: mpvObject.volume
        stepSize: 1

        onMoved: mpvObject.volume = value

        ToolTip.visible: hovered || pressed
        ToolTip.text: mpvObject.volume + "%"
        ToolTip.delay: 200

        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: slider.availableWidth
            height: 3
            radius: 1.5
            color: Kirigami.Theme.backgroundColor
            border.width: 1
            border.color: root.separatorColor

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                radius: 1.5
                color: mpvObject.muted ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.highlightColor
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            width: 10
            height: 10
            radius: 5
            color: slider.pressed ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

            Behavior on color {
                ColorAnimation { duration: 100 }
            }
        }
    }
}
