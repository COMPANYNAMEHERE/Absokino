import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Diagnostics

/**
 * DiagnosticsDialog - HDR and output diagnostics report
 */
Dialog {
    id: root

    required property MpvObject mpvObject
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    title: "HDR/Output Diagnostics"
    modal: true
    standardButtons: Dialog.Close
    closePolicy: Popup.CloseOnEscape

    width: 600
    height: 500

    property string reportText: ""

    onOpened: {
        HdrDiagnostics.mpvObject = mpvObject
        reportText = HdrDiagnostics.generateReport()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Header
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Diagnostic Report"
                font.weight: Font.Bold
                Layout.fillWidth: true
            }

            Button {
                text: "Refresh"
                icon.name: "view-refresh"
                onClicked: {
                    reportText = HdrDiagnostics.generateReport()
                }
            }

            Button {
                text: "Copy"
                icon.name: "edit-copy"
                onClicked: {
                    // Copy to clipboard
                    textArea.selectAll()
                    textArea.copy()
                    textArea.deselect()
                    copyConfirmation.visible = true
                    copyTimer.start()
                }
            }
        }

        // Copy confirmation
        Label {
            id: copyConfirmation
            text: "Copied to clipboard!"
            color: Kirigami.Theme.positiveTextColor
            visible: false

            Timer {
                id: copyTimer
                interval: 2000
                onTriggered: copyConfirmation.visible = false
            }
        }

        // Report text
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: textArea
                text: reportText
                readOnly: true
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                wrapMode: TextEdit.Wrap
                selectByMouse: true

                background: Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    border.width: 1
                    border.color: root.separatorColor
                    radius: Kirigami.Units.smallSpacing
                }
            }
        }

        // Important note
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: "Note: 'Display HDR: Unknown' is normal on Linux. This diagnostic cannot confirm if your display is actually receiving HDR signal. Check your monitor's OSD for HDR indicator."
            visible: true
        }
    }
}
