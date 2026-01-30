import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Diagnostics

/**
 * StatusBar - Top bar showing video quality/HDR indicators and codec info
 */
ToolBar {
    id: root

    required property MpvObject mpvObject
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    signal settingsClicked()
    signal diagnosticsClicked()

    // Set the mpv object for diagnostics
    Component.onCompleted: {
        HdrDiagnostics.mpvObject = mpvObject
    }

    // Helper component for status indicators
    component StatusIndicator: RowLayout {
        property string label
        property string value
        property bool highlighted: false
        property color highlightColor: Kirigami.Theme.highlightColor

        spacing: 2

        Label {
            text: label + ":"
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            opacity: 0.6
        }

        Rectangle {
            Layout.preferredWidth: valueLabel.implicitWidth + 8
            Layout.preferredHeight: valueLabel.implicitHeight + 4
            radius: 3
            color: highlighted ? highlightColor : "transparent"
            border.width: highlighted ? 0 : 1
            border.color: Kirigami.Theme.disabledTextColor

            Label {
                id: valueLabel
                anchors.centerIn: parent
                text: value
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                font.bold: highlighted
                color: highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            }
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Rectangle {
            anchors.bottom: parent.top
            width: parent.width
            height: 1
            color: root.separatorColor
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.smallSpacing
        anchors.rightMargin: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.largeSpacing

        // No video placeholder - inside the layout so it's not clipped
        Label {
            text: "No video loaded"
            opacity: 0.5
            visible: mpvObject.videoWidth <= 0
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        // HDR/Quality indicators
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: mpvObject.videoWidth > 0

            // Content HDR indicator
            StatusIndicator {
                label: "Content"
                value: mpvObject.contentIsHdr ? "HDR" : "SDR"
                highlighted: mpvObject.contentIsHdr
                highlightColor: Kirigami.Theme.positiveTextColor
            }

            StatusIndicator {
                label: "Output"
                value: {
                    let status = HdrDiagnostics.getCompactStatus()
                    return status.outputMode || "N/A"
                }
                highlighted: {
                    let status = HdrDiagnostics.getCompactStatus()
                    return status.outputMode === "Passthrough"
                }
                highlightColor: Kirigami.Theme.positiveTextColor
            }

            StatusIndicator {
                id: displayHdrIndicator
                label: "Display HDR"
                value: {
                    let status = HdrDiagnostics.getCompactStatus()
                    return status.displayHdr || "Unknown"
                }
                highlighted: {
                    let status = HdrDiagnostics.getCompactStatus()
                    return status.displayHdr === "Confirmed"
                }
                highlightColor: Kirigami.Theme.positiveTextColor

                ToolTip.text: "Click Diagnostics for details. 'Unknown' is normal on Linux."
                ToolTip.visible: displayHdrHover.hovered
                ToolTip.delay: 500

                HoverHandler {
                    id: displayHdrHover
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    onTapped: root.diagnosticsClicked()
                }
            }
        }

        // Separator
        Rectangle {
            visible: mpvObject.videoWidth > 0
            width: 1
            Layout.fillHeight: true
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            color: root.separatorColor
        }

        // Video info
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: mpvObject.videoWidth > 0

            // Codec
            Label {
                text: mpvObject.videoCodec.toUpperCase()
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.8
            }

            Label {
                text: "|"
                opacity: 0.4
            }

            // Resolution
            Label {
                text: mpvObject.videoWidth + "x" + mpvObject.videoHeight
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.8
            }

            Label {
                text: "|"
                opacity: 0.4
            }

            // FPS
            Label {
                text: mpvObject.fps.toFixed(2) + " fps"
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.8
            }

            Label {
                text: "|"
                opacity: 0.4
            }

            // Bit depth
            Label {
                text: mpvObject.bitDepth + "-bit"
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.8
            }

            Label {
                text: "|"
                opacity: 0.4
            }

            // Pixel format
            Label {
                text: mpvObject.pixelFormat
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.8
            }
        }

        Item { Layout.fillWidth: true }

        // Renderer info
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: mpvObject.videoWidth > 0

            // Hardware decoding
            Label {
                text: mpvObject.hwdecCurrent ? "HW: " + mpvObject.hwdecCurrent : "SW"
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                color: mpvObject.hwdecCurrent ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
                opacity: 0.8
            }

            Label {
                text: "|"
                opacity: 0.4
            }

            // Renderer
            Label {
                text: mpvObject.gpuApi || "OpenGL"
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.8
            }
        }

        // Settings/Diagnostics buttons
        ToolButton {
            icon.name: "help-about"
            display: AbstractButton.IconOnly
            ToolTip.text: "Run HDR/Output Diagnostics"
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            onClicked: root.diagnosticsClicked()
        }

        ToolButton {
            icon.name: "configure"
            display: AbstractButton.IconOnly
            ToolTip.text: "Settings"
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            onClicked: root.settingsClicked()
        }
    }

}
