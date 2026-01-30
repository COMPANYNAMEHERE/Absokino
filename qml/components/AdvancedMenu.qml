import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Player

/**
 * AdvancedMenu - Menu with advanced playback options
 */
Menu {
    id: root

    required property MpvObject mpvObject

    title: "Advanced"

    Menu {
        title: "Speed"

        MenuItem { text: "0.25x"; checkable: true; checked: Math.abs(mpvObject.speed - 0.25) < 0.01; onTriggered: mpvObject.speed = 0.25 }
        MenuItem { text: "0.5x"; checkable: true; checked: Math.abs(mpvObject.speed - 0.5) < 0.01; onTriggered: mpvObject.speed = 0.5 }
        MenuItem { text: "0.75x"; checkable: true; checked: Math.abs(mpvObject.speed - 0.75) < 0.01; onTriggered: mpvObject.speed = 0.75 }
        MenuItem { text: "1.0x (Normal)"; checkable: true; checked: Math.abs(mpvObject.speed - 1.0) < 0.01; onTriggered: mpvObject.speed = 1.0 }
        MenuItem { text: "1.25x"; checkable: true; checked: Math.abs(mpvObject.speed - 1.25) < 0.01; onTriggered: mpvObject.speed = 1.25 }
        MenuItem { text: "1.5x"; checkable: true; checked: Math.abs(mpvObject.speed - 1.5) < 0.01; onTriggered: mpvObject.speed = 1.5 }
        MenuItem { text: "1.75x"; checkable: true; checked: Math.abs(mpvObject.speed - 1.75) < 0.01; onTriggered: mpvObject.speed = 1.75 }
        MenuItem { text: "2.0x"; checkable: true; checked: Math.abs(mpvObject.speed - 2.0) < 0.01; onTriggered: mpvObject.speed = 2.0 }
    }

    Menu {
        title: "A-B Loop"

        MenuItem {
            text: mpvObject.loopA >= 0 ? "Set A (current: " + PlayerController.formatTime(mpvObject.loopA) + ")" : "Set A"
            onTriggered: mpvObject.setLoopA()
        }
        MenuItem {
            text: mpvObject.loopB >= 0 ? "Set B (current: " + PlayerController.formatTime(mpvObject.loopB) + ")" : "Set B"
            onTriggered: mpvObject.setLoopB()
        }
        MenuSeparator {}
        MenuItem {
            text: "Clear Loop"
            enabled: mpvObject.loopA >= 0 || mpvObject.loopB >= 0
            onTriggered: mpvObject.clearLoop()
        }
    }

    MenuSeparator {}

    MenuItem {
        text: "Frame Step Forward"
        shortcut: "."
        onTriggered: mpvObject.frameStep()
    }

    MenuItem {
        text: "Frame Step Backward"
        shortcut: ","
        onTriggered: mpvObject.frameBackStep()
    }
}
