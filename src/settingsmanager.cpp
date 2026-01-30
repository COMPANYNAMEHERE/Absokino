#include "settingsmanager.h"

SettingsManager *SettingsManager::s_instance = nullptr;

SettingsManager *SettingsManager::instance()
{
    if (!s_instance) {
        s_instance = new SettingsManager();
    }
    return s_instance;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("Absokino", "Absokino")
{
}

QString SettingsManager::hdrMode() const
{
    return m_settings.value("playback/hdrMode", "auto").toString();
}

void SettingsManager::setHdrMode(const QString &mode)
{
    if (hdrMode() != mode) {
        m_settings.setValue("playback/hdrMode", mode);
        emit hdrModeChanged();
    }
}

QString SettingsManager::hwdecMode() const
{
    return m_settings.value("playback/hwdecMode", "auto").toString();
}

void SettingsManager::setHwdecMode(const QString &mode)
{
    if (hwdecMode() != mode) {
        m_settings.setValue("playback/hwdecMode", mode);
        emit hwdecModeChanged();
    }
}

QString SettingsManager::rendererMode() const
{
    return m_settings.value("playback/rendererMode", "auto").toString();
}

void SettingsManager::setRendererMode(const QString &mode)
{
    if (rendererMode() != mode) {
        m_settings.setValue("playback/rendererMode", mode);
        emit rendererModeChanged();
    }
}

QString SettingsManager::fullscreenBehavior() const
{
    return m_settings.value("ui/fullscreenBehavior", "no_ui").toString();
}

void SettingsManager::setFullscreenBehavior(const QString &behavior)
{
    if (fullscreenBehavior() != behavior) {
        m_settings.setValue("ui/fullscreenBehavior", behavior);
        emit fullscreenBehaviorChanged();
    }
}

int SettingsManager::volume() const
{
    return m_settings.value("playback/volume", 100).toInt();
}

void SettingsManager::setVolume(int vol)
{
    int maxVolume = allowVolumeBoost() ? 150 : 100;
    vol = qBound(0, vol, maxVolume);
    if (volume() != vol) {
        m_settings.setValue("playback/volume", vol);
        emit volumeChanged();
    }
}

bool SettingsManager::allowVolumeBoost() const
{
    return m_settings.value("playback/allowVolumeBoost", false).toBool();
}

void SettingsManager::setAllowVolumeBoost(bool allow)
{
    if (allowVolumeBoost() != allow) {
        m_settings.setValue("playback/allowVolumeBoost", allow);
        emit allowVolumeBoostChanged();
    }

    if (!allow) {
        int currentVolume = m_settings.value("playback/volume", 100).toInt();
        if (currentVolume > 100) {
            m_settings.setValue("playback/volume", 100);
            emit volumeChanged();
        }
    }
}

QSize SettingsManager::windowSize() const
{
    return m_settings.value("ui/windowSize", QSize(1280, 720)).toSize();
}

void SettingsManager::setWindowSize(const QSize &size)
{
    if (windowSize() != size) {
        m_settings.setValue("ui/windowSize", size);
        emit windowSizeChanged();
    }
}

bool SettingsManager::windowMaximized() const
{
    return m_settings.value("ui/windowMaximized", false).toBool();
}

void SettingsManager::setWindowMaximized(bool maximized)
{
    if (windowMaximized() != maximized) {
        m_settings.setValue("ui/windowMaximized", maximized);
        emit windowMaximizedChanged();
    }
}

void SettingsManager::sync()
{
    m_settings.sync();
}
