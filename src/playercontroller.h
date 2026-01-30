#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QUrl>

class MpvObject;

/**
 * @brief PlayerController - Central controller for player state and actions
 *
 * This singleton manages the connection between QML and MpvObject,
 * handles file operations, and coordinates application-level player state.
 */
class PlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isFullscreen READ isFullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool libraryVisible READ libraryVisible WRITE setLibraryVisible NOTIFY libraryVisibleChanged)
    Q_PROPERTY(QString initialFile READ initialFile NOTIFY initialFileChanged)

public:
    static PlayerController *instance();

    bool isFullscreen() const { return m_isFullscreen; }
    void setFullscreen(bool fullscreen);

    bool libraryVisible() const { return m_libraryVisible; }
    void setLibraryVisible(bool visible);

    QString initialFile() const { return m_initialFile; }
    void setInitialFile(const QString &file);

    void setMpvObject(MpvObject *mpv) { m_mpvObject = mpv; }
    MpvObject *mpvObject() const { return m_mpvObject; }

public slots:
    void openFile();
    void openFileUrl(const QUrl &url);
    QString formatTime(double seconds) const;
    QString formatBitrate(double bps) const;

signals:
    void fullscreenChanged();
    void libraryVisibleChanged();
    void initialFileChanged();
    void fileOpened(const QString &path);
    void requestOpenFile();  // Signal to QML to open file dialog

private:
    explicit PlayerController(QObject *parent = nullptr);
    ~PlayerController() override = default;

    static PlayerController *s_instance;

    MpvObject *m_mpvObject = nullptr;
    bool m_isFullscreen = false;
    bool m_libraryVisible = false;
    QString m_initialFile;
};

#endif // PLAYERCONTROLLER_H
