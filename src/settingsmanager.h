#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QSize>

/**
 * @brief SettingsManager - Handles persistent application settings
 *
 * Manages settings for HDR mode, hardware decoding, renderer,
 * window geometry, volume, and other preferences.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

    // HDR Settings
    Q_PROPERTY(QString hdrMode READ hdrMode WRITE setHdrMode NOTIFY hdrModeChanged)

    // Hardware decoding
    Q_PROPERTY(QString hwdecMode READ hwdecMode WRITE setHwdecMode NOTIFY hwdecModeChanged)

    // Renderer
    Q_PROPERTY(QString rendererMode READ rendererMode WRITE setRendererMode NOTIFY rendererModeChanged)

    // Fullscreen behavior
    Q_PROPERTY(QString fullscreenBehavior READ fullscreenBehavior WRITE setFullscreenBehavior NOTIFY fullscreenBehaviorChanged)

    // Volume
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool allowVolumeBoost READ allowVolumeBoost WRITE setAllowVolumeBoost NOTIFY allowVolumeBoostChanged)

    // Window geometry
    Q_PROPERTY(QSize windowSize READ windowSize WRITE setWindowSize NOTIFY windowSizeChanged)
    Q_PROPERTY(bool windowMaximized READ windowMaximized WRITE setWindowMaximized NOTIFY windowMaximizedChanged)

public:
    static SettingsManager *instance();

    // HDR mode: "auto", "passthrough", "tonemap"
    QString hdrMode() const;
    void setHdrMode(const QString &mode);

    // Hardware decoding: "auto", "on", "off"
    QString hwdecMode() const;
    void setHwdecMode(const QString &mode);

    // Renderer: "auto", "vulkan", "opengl"
    QString rendererMode() const;
    void setRendererMode(const QString &mode);

    // Fullscreen behavior: "no_ui", "show_on_move"
    QString fullscreenBehavior() const;
    void setFullscreenBehavior(const QString &behavior);

    // Volume (0-100 or 0-150 with boost)
    int volume() const;
    void setVolume(int vol);
    bool allowVolumeBoost() const;
    void setAllowVolumeBoost(bool allow);

    // Window geometry
    QSize windowSize() const;
    void setWindowSize(const QSize &size);

    bool windowMaximized() const;
    void setWindowMaximized(bool maximized);

public slots:
    void sync();

signals:
    void hdrModeChanged();
    void hwdecModeChanged();
    void rendererModeChanged();
    void fullscreenBehaviorChanged();
    void volumeChanged();
    void allowVolumeBoostChanged();
    void windowSizeChanged();
    void windowMaximizedChanged();

private:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() override = default;

    static SettingsManager *s_instance;
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
