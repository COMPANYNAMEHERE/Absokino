import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Settings
import Absokino.Diagnostics

/**
 * SettingsDialog - Application settings
 */
Dialog {
    id: root

    required property MpvObject mpvObject
    readonly property color separatorColor: Kirigami.ColorUtils.linearInterpolation(
        Kirigami.Theme.backgroundColor,
        Kirigami.Theme.textColor,
        Kirigami.Theme.frameContrast
    )

    title: "Settings"
    modal: true
    standardButtons: Dialog.Close
    closePolicy: Popup.CloseOnEscape

    width: 500
    height: 500

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
        border.width: 1
        border.color: root.separatorColor
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        TabBar {
            id: tabs
            Layout.fillWidth: true

            TabButton { text: "Playback" }
            TabButton { text: "Audio" }
            TabButton { text: "UI" }
            TabButton { text: "Diagnostics" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            // Playback tab
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScrollView {
                    anchors.fill: parent
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: Kirigami.Units.largeSpacing

                        // HDR Settings
                        GroupBox {
                            title: "HDR Output Mode"
                            Layout.fillWidth: true

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    text: "Controls how HDR content is processed for output."
                                    wrapMode: Text.WordWrap
                                    opacity: 0.7
                                    Layout.fillWidth: true
                                }

                                RadioButton {
                                    text: "Auto (Passthrough preferred)"
                                    checked: Settings.hdrMode === "auto"
                                    onClicked: {
                                        Settings.hdrMode = "auto"
                                        mpvObject.setHdrMode("auto")
                                    }

                                    ToolTip.text: "Prefer HDR passthrough when possible, with automatic fallback"
                                    ToolTip.visible: hovered
                                    ToolTip.delay: 500
                                }

                                RadioButton {
                                    text: "Passthrough (HDR output)"
                                    checked: Settings.hdrMode === "passthrough"
                                    onClicked: {
                                        Settings.hdrMode = "passthrough"
                                        mpvObject.setHdrMode("passthrough")
                                    }

                                    ToolTip.text: "Send HDR content directly to display without tone-mapping"
                                    ToolTip.visible: hovered
                                    ToolTip.delay: 500
                                }

                                RadioButton {
                                    text: "Tone-map to SDR"
                                    checked: Settings.hdrMode === "tonemap"
                                    onClicked: {
                                        Settings.hdrMode = "tonemap"
                                        mpvObject.setHdrMode("tonemap")
                                    }

                                    ToolTip.text: "Convert HDR content to SDR using tone-mapping"
                                    ToolTip.visible: hovered
                                    ToolTip.delay: 500
                                }
                            }
                        }

                        // Hardware Decoding
                        GroupBox {
                            title: "Hardware Decoding"
                            Layout.fillWidth: true

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    text: "Use GPU for video decoding when available."
                                    wrapMode: Text.WordWrap
                                    opacity: 0.7
                                    Layout.fillWidth: true
                                }

                                RadioButton {
                                    text: "Auto (Recommended)"
                                    checked: Settings.hwdecMode === "auto"
                                    onClicked: {
                                        Settings.hwdecMode = "auto"
                                        mpvObject.setHwdecMode("auto")
                                    }
                                }

                                RadioButton {
                                    text: "Always On"
                                    checked: Settings.hwdecMode === "on"
                                    onClicked: {
                                        Settings.hwdecMode = "on"
                                        mpvObject.setHwdecMode("on")
                                    }
                                }

                                RadioButton {
                                    text: "Off (Software only)"
                                    checked: Settings.hwdecMode === "off"
                                    onClicked: {
                                        Settings.hwdecMode = "off"
                                        mpvObject.setHwdecMode("off")
                                    }
                                }
                            }
                        }

                        // Renderer
                        GroupBox {
                            title: "Renderer"
                            Layout.fillWidth: true

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    text: "Graphics API for video rendering. Changes require restart."
                                    wrapMode: Text.WordWrap
                                    opacity: 0.7
                                    Layout.fillWidth: true
                                }

                                RadioButton {
                                    text: "Auto"
                                    checked: Settings.rendererMode === "auto"
                                    onClicked: Settings.rendererMode = "auto"
                                }

                                RadioButton {
                                    text: "Vulkan (Better HDR support)"
                                    checked: Settings.rendererMode === "vulkan"
                                    onClicked: Settings.rendererMode = "vulkan"
                                }

                                RadioButton {
                                    text: "OpenGL"
                                    checked: Settings.rendererMode === "opengl"
                                    onClicked: Settings.rendererMode = "opengl"
                                }
                            }
                        }
                    }
                }
            }

            // Audio tab
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScrollView {
                    anchors.fill: parent
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: Kirigami.Units.largeSpacing

                        GroupBox {
                            title: "Audio"
                            Layout.fillWidth: true

                            ColumnLayout {
                                anchors.fill: parent

                                Label {
                                    text: "Control volume behavior."
                                    wrapMode: Text.WordWrap
                                    opacity: 0.7
                                    Layout.fillWidth: true
                                }

                                CheckBox {
                                    text: "Allow volume above 100% (boost)"
                                    checked: Settings.allowVolumeBoost
                                    onToggled: {
                                        Settings.allowVolumeBoost = checked
                                        if (!checked && mpvObject.volume > 100) {
                                            mpvObject.volume = 100
                                        }
                                    }

                                    ToolTip.text: "Enables volume up to 150%. Higher levels may introduce distortion."
                                    ToolTip.visible: hovered
                                    ToolTip.delay: 500
                                }
                            }
                        }
                    }
                }
            }

            // UI tab
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScrollView {
                    anchors.fill: parent
                    clip: true

                    ColumnLayout {
                        width: parent.width
                        spacing: Kirigami.Units.largeSpacing

                        GroupBox {
                            title: "Fullscreen Behavior"
                            Layout.fillWidth: true

                            ColumnLayout {
                                anchors.fill: parent

                                RadioButton {
                                    text: "No UI, ignore clicks (Default)"
                                    checked: Settings.fullscreenBehavior === "no_ui"
                                    onClicked: Settings.fullscreenBehavior = "no_ui"

                                    ToolTip.text: "In fullscreen: no controls shown, clicks do nothing. Use Space for pause, Esc to exit."
                                    ToolTip.visible: hovered
                                    ToolTip.delay: 500
                                }

                                RadioButton {
                                    text: "Show UI on mouse move"
                                    checked: Settings.fullscreenBehavior === "show_on_move"
                                    onClicked: Settings.fullscreenBehavior = "show_on_move"

                                    ToolTip.text: "Show controls briefly when mouse moves (not yet implemented)"
                                    ToolTip.visible: hovered
                                    ToolTip.delay: 500
                                    enabled: false  // TODO: Implement this feature
                                }
                            }
                        }
                    }
                }
            }

            // Diagnostics tab
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.largeSpacing

                    Label {
                        text: "Run diagnostics to capture HDR and output information."
                        wrapMode: Text.WordWrap
                        opacity: 0.7
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Run HDR/Output Diagnostics..."
                        icon.name: "help-about"
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: diagnosticsDialog.open()
                    }
                }
            }
        }
    }

    // Reference to diagnostics dialog (injected from parent)
    property var diagnosticsDialog
}
