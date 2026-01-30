#include "playercontroller.h"
#include "mpvobject.h"
#include "recentfilesmodel.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <cmath>

PlayerController *PlayerController::s_instance = nullptr;

PlayerController *PlayerController::instance()
{
    if (!s_instance) {
        s_instance = new PlayerController();
    }
    return s_instance;
}

PlayerController::PlayerController(QObject *parent)
    : QObject(parent)
{
}

void PlayerController::setFullscreen(bool fullscreen)
{
    if (m_isFullscreen != fullscreen) {
        m_isFullscreen = fullscreen;
        emit fullscreenChanged();
    }
}

void PlayerController::setLibraryVisible(bool visible)
{
    if (m_libraryVisible != visible) {
        m_libraryVisible = visible;
        emit libraryVisibleChanged();
    }
}

void PlayerController::setInitialFile(const QString &file)
{
    m_initialFile = file;
    emit initialFileChanged();
}

void PlayerController::openFile()
{
    emit requestOpenFile();
}

void PlayerController::openFileUrl(const QUrl &url)
{
    QString path = url.toLocalFile();
    if (path.isEmpty()) {
        return;
    }

    // Add to recent files
    RecentFilesModel::instance()->addFile(path);

    // Load in mpv
    if (m_mpvObject) {
        m_mpvObject->loadFile(path);
    }

    emit fileOpened(path);
}

QString PlayerController::formatTime(double seconds) const
{
    if (std::isnan(seconds) || seconds < 0) {
        return "--:--";
    }

    int totalSeconds = static_cast<int>(seconds);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes)
            .arg(secs, 2, 10, QChar('0'));
    }
}

QString PlayerController::formatBitrate(double bps) const
{
    if (bps <= 0) {
        return QString();
    }

    if (bps >= 1000000) {
        return QString("%1 Mbps").arg(bps / 1000000.0, 0, 'f', 1);
    } else if (bps >= 1000) {
        return QString("%1 kbps").arg(bps / 1000.0, 0, 'f', 0);
    } else {
        return QString("%1 bps").arg(bps, 0, 'f', 0);
    }
}
