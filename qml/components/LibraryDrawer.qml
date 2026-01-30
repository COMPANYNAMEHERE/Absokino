import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import Absokino.Models
import Absokino.Player

/**
 * LibraryDrawer - Slide-in panel showing recent files
 */
Rectangle {
    id: root

    width: 280
    color: Kirigami.Theme.backgroundColor

    signal fileSelected(string path)

    // Right border
    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: Kirigami.Theme.separatorColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing

        // Header
        RowLayout {
            Layout.fillWidth: true

            Kirigami.Heading {
                text: "Library"
                level: 4
                Layout.fillWidth: true
            }

            ToolButton {
                icon.name: "edit-clear-history"
                display: AbstractButton.IconOnly
                ToolTip.text: "Clear History"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                enabled: RecentFiles.count > 0
                onClicked: clearConfirmDialog.open()
            }

            ToolButton {
                icon.name: "window-close"
                display: AbstractButton.IconOnly
                ToolTip.text: "Close Library"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                onClicked: PlayerController.libraryVisible = false
            }
        }

        // Search field (optional future enhancement)
        // TextField {
        //     Layout.fillWidth: true
        //     placeholderText: "Search..."
        //     visible: RecentFiles.count > 10
        // }

        // Recent files list
        ListView {
            id: fileList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: RecentFiles

            delegate: ItemDelegate {
                width: fileList.width
                height: contentColumn.implicitHeight + Kirigami.Units.smallSpacing * 2

                background: Rectangle {
                    color: parent.hovered ? Kirigami.Theme.hoverColor : "transparent"
                    radius: Kirigami.Units.smallSpacing
                }

                contentItem: ColumnLayout {
                    id: contentColumn
                    spacing: 2

                    Label {
                        text: model.displayName
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        font.weight: Font.Medium
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Label {
                            text: model.fileSize
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                            opacity: 0.6
                        }

                        Label {
                            text: "•"
                            opacity: 0.4
                        }

                        Label {
                            text: model.lastPlayed
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                            opacity: 0.6
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                }

                onClicked: {
                    root.fileSelected(model.path)
                }

                // Context menu for individual items
                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onTapped: itemMenu.popup()
                }

                Menu {
                    id: itemMenu

                    MenuItem {
                        text: "Play"
                        icon.name: "media-playback-start"
                        onTriggered: root.fileSelected(model.path)
                    }

                    MenuItem {
                        text: "Remove from History"
                        icon.name: "edit-delete"
                        onTriggered: RecentFiles.removeFile(model.path)
                    }
                }
            }

            // Empty state
            Label {
                anchors.centerIn: parent
                text: "No recent files"
                opacity: 0.5
                visible: RecentFiles.count === 0
            }

            ScrollBar.vertical: ScrollBar {
                policy: fileList.contentHeight > fileList.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            }
        }

        // Footer with count
        Label {
            Layout.fillWidth: true
            text: RecentFiles.count + " files in history"
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            opacity: 0.5
            horizontalAlignment: Text.AlignHCenter
            visible: RecentFiles.count > 0
        }
    }

    // Clear confirmation dialog
    Dialog {
        id: clearConfirmDialog
        title: "Clear History"
        standardButtons: Dialog.Yes | Dialog.No
        modal: true

        Label {
            text: "Are you sure you want to clear all recent files?"
        }

        onAccepted: RecentFiles.clearAll()
    }
}
