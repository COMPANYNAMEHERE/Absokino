import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

import Absokino.Mpv

/**
 * VideoArea - Container for the video display
 *
 * This component wraps the MpvObject and handles mouse interactions.
 * In fullscreen mode, it ignores single clicks as specified.
 */
Item {
    id: root

    required property MpvObject mpvObject
    property bool fullscreenMode: false

    signal doubleClicked()
    signal rightClicked(var mouse)
    signal singleClicked()

    // Background
    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
    }

    // Video rendering surface
    Item {
        id: videoContainer
        anchors.fill: parent

        // The MpvObject renders here via FBO
        // We reparent the mpvObject to this container
        Component.onCompleted: {
            if (mpvObject) {
                mpvObject.parent = videoContainer
                mpvObject.anchors.fill = videoContainer
                mpvObject.visible = true
            }
        }
    }

    // Mouse handling
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: !fullscreenMode

        property real lastClickTime: 0
        property real doubleClickThreshold: 300  // ms

        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                if (!fullscreenMode) {
                    root.rightClicked(mouse)
                }
                return
            }

            // In fullscreen mode, single clicks do nothing (as specified)
            if (fullscreenMode) {
                // Check for double-click manually
                let currentTime = Date.now()
                if (currentTime - lastClickTime < doubleClickThreshold) {
                    root.doubleClicked()
                    lastClickTime = 0
                } else {
                    lastClickTime = currentTime
                }
                return
            }

            root.singleClicked()
        }

        onDoubleClicked: {
            // Only emit if not in fullscreen (fullscreen handles it via manual detection above)
            if (!fullscreenMode) {
                root.doubleClicked()
            }
        }
    }

    // "No video" placeholder
    Item {
        anchors.centerIn: parent
        visible: !mpvObject || (!mpvObject.playing && mpvObject.duration <= 0)
        opacity: 0.5

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
}
