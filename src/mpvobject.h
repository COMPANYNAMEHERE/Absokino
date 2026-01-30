#ifndef MPVOBJECT_H
#define MPVOBJECT_H

#include <QQuickFramebufferObject>
#include <QThread>
#include <QMutex>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvRenderer;

/**
 * @brief MpvObject - Qt Quick item that renders mpv video via libmpv render API
 *
 * This class integrates libmpv directly using the render API, not via IPC.
 * It creates an OpenGL/Vulkan context for rendering video frames into a Qt FBO.
 */
class MpvObject : public QQuickFramebufferObject
{
    Q_OBJECT

    // Playback state properties
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(double position READ position NOTIFY positionChanged)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double percentPos READ percentPos NOTIFY percentPosChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY speedChanged)

    // Video information properties
    Q_PROPERTY(int videoWidth READ videoWidth NOTIFY videoParamsChanged)
    Q_PROPERTY(int videoHeight READ videoHeight NOTIFY videoParamsChanged)
    Q_PROPERTY(double fps READ fps NOTIFY videoParamsChanged)
    Q_PROPERTY(QString videoCodec READ videoCodec NOTIFY videoParamsChanged)
    Q_PROPERTY(QString audioCodec READ audioCodec NOTIFY audioParamsChanged)
    Q_PROPERTY(QString pixelFormat READ pixelFormat NOTIFY videoParamsChanged)
    Q_PROPERTY(int bitDepth READ bitDepth NOTIFY videoParamsChanged)
    Q_PROPERTY(QString colorPrimaries READ colorPrimaries NOTIFY videoParamsChanged)
    Q_PROPERTY(QString colorTransfer READ colorTransfer NOTIFY videoParamsChanged)
    Q_PROPERTY(QString colorMatrix READ colorMatrix NOTIFY videoParamsChanged)
    Q_PROPERTY(QString hwdecCurrent READ hwdecCurrent NOTIFY hwdecChanged)
    Q_PROPERTY(QString voBackend READ voBackend NOTIFY rendererChanged)
    Q_PROPERTY(QString gpuApi READ gpuApi NOTIFY rendererChanged)

    // HDR metadata
    Q_PROPERTY(bool contentIsHdr READ contentIsHdr NOTIFY hdrInfoChanged)
    Q_PROPERTY(double maxCll READ maxCll NOTIFY hdrInfoChanged)
    Q_PROPERTY(double maxFall READ maxFall NOTIFY hdrInfoChanged)

    // Track information
    Q_PROPERTY(QVariantList audioTracks READ audioTracks NOTIFY tracksChanged)
    Q_PROPERTY(QVariantList subtitleTracks READ subtitleTracks NOTIFY tracksChanged)
    Q_PROPERTY(int currentAudioTrack READ currentAudioTrack NOTIFY currentAudioTrackChanged)
    Q_PROPERTY(int currentSubtitleTrack READ currentSubtitleTrack NOTIFY currentSubtitleTrackChanged)

    // Chapter information
    Q_PROPERTY(QVariantList chapters READ chapters NOTIFY chaptersChanged)
    Q_PROPERTY(int currentChapter READ currentChapter NOTIFY currentChapterChanged)

    // File information
    Q_PROPERTY(QString filename READ filename NOTIFY filenameChanged)
    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaTitleChanged)

    // A-B Loop
    Q_PROPERTY(double loopA READ loopA NOTIFY loopChanged)
    Q_PROPERTY(double loopB READ loopB NOTIFY loopChanged)

    // Error state
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorOccurred)

public:
    explicit MpvObject(QQuickItem *parent = nullptr);
    ~MpvObject() override;

    Renderer *createRenderer() const override;

    mpv_handle *mpvHandle() const { return m_mpv; }
    mpv_render_context *renderContext() const { return m_renderCtx; }

    // Property getters
    bool playing() const { return m_playing; }
    bool paused() const { return m_paused; }
    double position() const { return m_position; }
    double duration() const { return m_duration; }
    double percentPos() const { return m_percentPos; }
    int volume() const { return m_volume; }
    bool muted() const { return m_muted; }
    double speed() const { return m_speed; }

    int videoWidth() const { return m_videoWidth; }
    int videoHeight() const { return m_videoHeight; }
    double fps() const { return m_fps; }
    QString videoCodec() const { return m_videoCodec; }
    QString audioCodec() const { return m_audioCodec; }
    QString pixelFormat() const { return m_pixelFormat; }
    int bitDepth() const { return m_bitDepth; }
    QString colorPrimaries() const { return m_colorPrimaries; }
    QString colorTransfer() const { return m_colorTransfer; }
    QString colorMatrix() const { return m_colorMatrix; }
    QString hwdecCurrent() const { return m_hwdecCurrent; }
    QString voBackend() const { return m_voBackend; }
    QString gpuApi() const { return m_gpuApi; }

    bool contentIsHdr() const { return m_contentIsHdr; }
    double maxCll() const { return m_maxCll; }
    double maxFall() const { return m_maxFall; }

    QVariantList audioTracks() const { return m_audioTracks; }
    QVariantList subtitleTracks() const { return m_subtitleTracks; }
    int currentAudioTrack() const { return m_currentAudioTrack; }
    int currentSubtitleTrack() const { return m_currentSubtitleTrack; }

    QVariantList chapters() const { return m_chapters; }
    int currentChapter() const { return m_currentChapter; }

    QString filename() const { return m_filename; }
    QString mediaTitle() const { return m_mediaTitle; }

    double loopA() const { return m_loopA; }
    double loopB() const { return m_loopB; }

    QString lastError() const { return m_lastError; }

    // Property setters
    void setVolume(int vol);
    void setMuted(bool muted);
    void setSpeed(double speed);

public slots:
    // Playback control
    void loadFile(const QString &path);
    void play();
    void pause();
    void stop();
    void togglePause();
    void seek(double seconds);
    void seekAbsolute(double seconds);
    void seekPercent(double percent);

    // Track selection
    void setAudioTrack(int id);
    void setSubtitleTrack(int id);
    void loadSubtitleFile(const QString &path);

    // Chapter navigation
    void setChapter(int index);
    void nextChapter();
    void previousChapter();

    // Frame stepping
    void frameStep();
    void frameBackStep();

    // A-B Loop
    void setLoopA();
    void setLoopB();
    void clearLoop();

    // Configuration
    void setHdrMode(const QString &mode);
    void setHwdecMode(const QString &mode);
    void setRendererMode(const QString &mode);

    // Get mpv property directly (for diagnostics)
    QVariant getMpvProperty(const QString &name) const;
    QString getMpvVersion() const;

signals:
    void playingChanged();
    void pausedChanged();
    void positionChanged();
    void durationChanged();
    void percentPosChanged();
    void volumeChanged();
    void mutedChanged();
    void speedChanged();

    void videoParamsChanged();
    void audioParamsChanged();
    void hwdecChanged();
    void rendererChanged();
    void hdrInfoChanged();

    void tracksChanged();
    void currentAudioTrackChanged();
    void currentSubtitleTrackChanged();

    void chaptersChanged();
    void currentChapterChanged();

    void filenameChanged();
    void mediaTitleChanged();

    void loopChanged();

    void errorOccurred(const QString &error);
    void fileLoaded();
    void endOfFile();

private slots:
    void onMpvEvents();
    void handleMpvEvent(mpv_event *event);

private:
    void initializeMpv();
    void initializeRenderContext();
    void setupPropertyObservers();
    void configureHdrOptions(const QString &mode);
    void updateVideoParams();
    void updateTracks();
    void updateChapters();
    void checkHdrContent();

    void setMpvOption(const QString &name, const QVariant &value);
    void setMpvProperty(const QString &name, const QVariant &value);
    QVariant getMpvPropertyVariant(const QString &name) const;

    static void onUpdate(void *ctx);
    static void onWakeup(void *ctx);

    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_renderCtx = nullptr;

    // Playback state
    bool m_playing = false;
    bool m_paused = false;
    double m_position = 0.0;
    double m_duration = 0.0;
    double m_percentPos = 0.0;
    int m_volume = 100;
    bool m_muted = false;
    double m_speed = 1.0;

    // Video params
    int m_videoWidth = 0;
    int m_videoHeight = 0;
    double m_fps = 0.0;
    QString m_videoCodec;
    QString m_audioCodec;
    QString m_pixelFormat;
    int m_bitDepth = 8;
    QString m_colorPrimaries;
    QString m_colorTransfer;
    QString m_colorMatrix;
    QString m_hwdecCurrent;
    QString m_voBackend;
    QString m_gpuApi;

    // HDR info
    bool m_contentIsHdr = false;
    double m_maxCll = 0.0;
    double m_maxFall = 0.0;

    // Tracks
    QVariantList m_audioTracks;
    QVariantList m_subtitleTracks;
    int m_currentAudioTrack = 0;
    int m_currentSubtitleTrack = 0;

    // Chapters
    QVariantList m_chapters;
    int m_currentChapter = -1;

    // File info
    QString m_filename;
    QString m_mediaTitle;

    // A-B Loop
    double m_loopA = -1.0;
    double m_loopB = -1.0;

    // Error
    QString m_lastError;

    friend class MpvRenderer;
};

#endif // MPVOBJECT_H
