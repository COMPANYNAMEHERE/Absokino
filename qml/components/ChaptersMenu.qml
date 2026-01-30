import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

import Absokino.Mpv
import Absokino.Player

/**
 * ChaptersMenu - Menu component for chapter navigation
 */
Menu {
    id: root

    required property MpvObject mpvObject

    title: "Chapters"
    enabled: mpvObject.chapters.length > 0

    Instantiator {
        model: mpvObject.chapters

        MenuItem {
            text: {
                let chapter = mpvObject.chapters[index]
                let title = chapter.title || ("Chapter " + (index + 1))
                let time = PlayerController.formatTime(chapter.time)
                return time + " - " + title
            }
            checkable: true
            checked: mpvObject.currentChapter === index
            onTriggered: mpvObject.setChapter(index)
        }

        onObjectAdded: (index, object) => root.insertItem(index, object)
        onObjectRemoved: (index, object) => root.removeItem(object)
    }
}
