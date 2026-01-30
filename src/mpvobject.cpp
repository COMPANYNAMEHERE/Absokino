#include "mpvobject.h"
#include "mpvrenderer.h"
#include "settingsmanager.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <stdexcept>

namespace {

void checkMpvError(int status)
{
    if (status < 0) {
        throw std::runtime_error(mpv_error_string(status));
    }
}

QVariant nodeToVariant(const mpv_node *node)
{
    switch (node->format) {
    case MPV_FORMAT_STRING:
        return QString::fromUtf8(node->u.string);
    case MPV_FORMAT_FLAG:
        return static_cast<bool>(node->u.flag);
    case MPV_FORMAT_INT64:
        return static_cast<qlonglong>(node->u.int64);
    case MPV_FORMAT_DOUBLE:
        return node->u.double_;
    case MPV_FORMAT_NODE_ARRAY: {
        QVariantList list;
        for (int i = 0; i < node->u.list->num; ++i) {
            list.append(nodeToVariant(&node->u.list->values[i]));
        }
        return list;
    }
    case MPV_FORMAT_NODE_MAP: {
        QVariantMap map;
        for (int i = 0; i < node->u.list->num; ++i) {
            map.insert(QString::fromUtf8(node->u.list->keys[i]),
                       nodeToVariant(&node->u.list->values[i]));
        }
        return map;
    }
    default:
        return QVariant();
    }
}

} // anonymous namespace

MpvObject::MpvObject(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    initializeMpv();

    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        if (window) {
            connect(window, &QQuickWindow::beforeSynchronizing, this, [this]() {
                // Sync state before rendering
            }, Qt::DirectConnection);
        }
    });
}

MpvObject::~MpvObject()
{
    if (m_renderCtx) {
        mpv_render_context_free(m_renderCtx);
    }
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
    }
}

void MpvObject::initializeMpv()
{
    m_mpv = mpv_create();
    if (!m_mpv) {
        m_lastError = "Failed to create mpv instance";
        emit errorOccurred(m_lastError);
        return;
    }

    try {
        // ====== CORE MPV OPTIONS ======
        // These are set before mpv_initialize() as required by libmpv

        // Terminal and logging
        setMpvOption("terminal", false);
        setMpvOption("msg-level", "all=warn");

        // Video output - use libmpv render API
        setMpvOption("vo", "libmpv");

        // ====== HARDWARE DECODING ======
        // Default: auto (let mpv choose the best available)
        QString hwdecMode = SettingsManager::instance()->hwdecMode();
        if (hwdecMode == "on") {
            setMpvOption("hwdec", "auto-safe");
        } else if (hwdecMode == "off") {
            setMpvOption("hwdec", "no");
        } else {
            // Auto mode - prefer hardware decoding with safe fallback
            setMpvOption("hwdec", "auto-safe");
        }

        // ====== HDR CONFIGURATION ======
        // Goal: Prefer HDR passthrough when possible on Linux/Wayland
        QString hdrMode = SettingsManager::instance()->hdrMode();
        configureHdrOptions(hdrMode);

        // ====== RENDERER CONFIGURATION ======
        // Prefer Vulkan for HDR passthrough support, fall back to OpenGL
        QString rendererMode = SettingsManager::instance()->rendererMode();
        if (rendererMode == "vulkan") {
            setMpvOption("gpu-api", "vulkan");
        } else if (rendererMode == "opengl") {
            setMpvOption("gpu-api", "opengl");
        } else {
            // Auto: let mpv choose (typically prefers OpenGL in Qt context)
            // Note: Qt's OpenGL context means we're limited to OpenGL here
        }

        // ====== AUDIO ======
        setMpvOption("audio-display", "no");  // Don't show album art in video

        // ====== SUBTITLES ======
        setMpvOption("sub-auto", "fuzzy");    // Auto-load subtitles
        setMpvOption("sub-visibility", true);

        // ====== PLAYBACK ======
        setMpvOption("keep-open", "yes");     // Don't close at end of file
        setMpvOption("idle", "yes");          // Stay running when idle

        // Initialize mpv
        checkMpvError(mpv_initialize(m_mpv));

        // Set up property observers after initialization
        setupPropertyObservers();

        // Set up event handling
        mpv_set_wakeup_callback(m_mpv, onWakeup, this);

    } catch (const std::exception &e) {
        m_lastError = QString("mpv initialization failed: %1").arg(e.what());
        emit errorOccurred(m_lastError);
        if (m_mpv) {
            mpv_terminate_destroy(m_mpv);
            m_mpv = nullptr;
        }
    }
}

void MpvObject::configureHdrOptions(const QString &mode)
{
    if (mode == "passthrough") {
        // Prefer HDR passthrough - don't tone map
        setMpvOption("target-trc", "auto");
        setMpvOption("target-prim", "auto");
        setMpvOption("tone-mapping", "clip");  // Minimal processing
        setMpvOption("hdr-compute-peak", "no");
        setMpvOption("target-colorspace-hint", "yes");  // Important for Wayland HDR
    } else if (mode == "tonemap") {
        // Force tone mapping to SDR
        setMpvOption("target-trc", "auto");
        setMpvOption("tone-mapping", "hable");
        setMpvOption("hdr-compute-peak", "yes");
        setMpvOption("target-colorspace-hint", "no");
    } else {
        // Auto mode: prefer passthrough but let mpv decide
        setMpvOption("target-trc", "auto");
        setMpvOption("target-prim", "auto");
        setMpvOption("tone-mapping", "auto");
        setMpvOption("hdr-compute-peak", "auto");
        setMpvOption("target-colorspace-hint", "yes");
    }
}

void MpvObject::setupPropertyObservers()
{
    // Observe playback state
    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "percent-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "volume", MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "mute", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "speed", MPV_FORMAT_DOUBLE);

    // Observe video parameters
    mpv_observe_property(m_mpv, 0, "video-params", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 0, "video-format", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "video-codec", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "audio-codec", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "estimated-vf-fps", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "container-fps", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "hwdec-current", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "current-vo", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "gpu-api", MPV_FORMAT_STRING);

    // Observe tracks
    mpv_observe_property(m_mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 0, "aid", MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "sid", MPV_FORMAT_INT64);

    // Observe chapters
    mpv_observe_property(m_mpv, 0, "chapter-list", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 0, "chapter", MPV_FORMAT_INT64);

    // Observe file info
    mpv_observe_property(m_mpv, 0, "filename", MPV_FORMAT_STRING);
    mpv_observe_property(m_mpv, 0, "media-title", MPV_FORMAT_STRING);

    // Observe A-B loop
    mpv_observe_property(m_mpv, 0, "ab-loop-a", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "ab-loop-b", MPV_FORMAT_DOUBLE);
}

void MpvObject::onWakeup(void *ctx)
{
    MpvObject *self = static_cast<MpvObject *>(ctx);
    QMetaObject::invokeMethod(self, "onMpvEvents", Qt::QueuedConnection);
}

void MpvObject::onMpvEvents()
{
    if (!m_mpv) return;

    while (m_mpv) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handleMpvEvent(event);
    }
}

void MpvObject::handleMpvEvent(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = static_cast<mpv_event_property *>(event->data);
        QString propName = QString::fromUtf8(prop->name);

        if (propName == "pause" && prop->format == MPV_FORMAT_FLAG) {
            m_paused = *static_cast<int *>(prop->data);
            m_playing = !m_paused && m_duration > 0;
            emit pausedChanged();
            emit playingChanged();
        } else if (propName == "time-pos" && prop->format == MPV_FORMAT_DOUBLE) {
            m_position = *static_cast<double *>(prop->data);
            emit positionChanged();
        } else if (propName == "duration" && prop->format == MPV_FORMAT_DOUBLE) {
            m_duration = *static_cast<double *>(prop->data);
            emit durationChanged();
        } else if (propName == "percent-pos" && prop->format == MPV_FORMAT_DOUBLE) {
            m_percentPos = *static_cast<double *>(prop->data);
            emit percentPosChanged();
        } else if (propName == "volume" && prop->format == MPV_FORMAT_INT64) {
            m_volume = static_cast<int>(*static_cast<int64_t *>(prop->data));
            emit volumeChanged();
        } else if (propName == "mute" && prop->format == MPV_FORMAT_FLAG) {
            m_muted = *static_cast<int *>(prop->data);
            emit mutedChanged();
        } else if (propName == "speed" && prop->format == MPV_FORMAT_DOUBLE) {
            m_speed = *static_cast<double *>(prop->data);
            emit speedChanged();
        } else if (propName == "video-params" && prop->format == MPV_FORMAT_NODE) {
            updateVideoParams();
        } else if (propName == "video-codec" && prop->format == MPV_FORMAT_STRING) {
            m_videoCodec = QString::fromUtf8(*static_cast<char **>(prop->data));
            emit videoParamsChanged();
        } else if (propName == "audio-codec" && prop->format == MPV_FORMAT_STRING) {
            m_audioCodec = QString::fromUtf8(*static_cast<char **>(prop->data));
            emit audioParamsChanged();
        } else if (propName == "estimated-vf-fps" && prop->format == MPV_FORMAT_DOUBLE) {
            m_fps = *static_cast<double *>(prop->data);
            emit videoParamsChanged();
        } else if (propName == "hwdec-current" && prop->format == MPV_FORMAT_STRING) {
            char *val = *static_cast<char **>(prop->data);
            m_hwdecCurrent = val ? QString::fromUtf8(val) : QString();
            emit hwdecChanged();
        } else if (propName == "current-vo" && prop->format == MPV_FORMAT_STRING) {
            char *val = *static_cast<char **>(prop->data);
            m_voBackend = val ? QString::fromUtf8(val) : QString();
            emit rendererChanged();
        } else if (propName == "gpu-api" && prop->format == MPV_FORMAT_STRING) {
            char *val = *static_cast<char **>(prop->data);
            m_gpuApi = val ? QString::fromUtf8(val) : QString();
            emit rendererChanged();
        } else if (propName == "track-list" && prop->format == MPV_FORMAT_NODE) {
            updateTracks();
        } else if (propName == "aid" && prop->format == MPV_FORMAT_INT64) {
            m_currentAudioTrack = static_cast<int>(*static_cast<int64_t *>(prop->data));
            emit currentAudioTrackChanged();
        } else if (propName == "sid" && prop->format == MPV_FORMAT_INT64) {
            m_currentSubtitleTrack = static_cast<int>(*static_cast<int64_t *>(prop->data));
            emit currentSubtitleTrackChanged();
        } else if (propName == "chapter-list" && prop->format == MPV_FORMAT_NODE) {
            updateChapters();
        } else if (propName == "chapter" && prop->format == MPV_FORMAT_INT64) {
            m_currentChapter = static_cast<int>(*static_cast<int64_t *>(prop->data));
            emit currentChapterChanged();
        } else if (propName == "filename" && prop->format == MPV_FORMAT_STRING) {
            char *val = *static_cast<char **>(prop->data);
            m_filename = val ? QString::fromUtf8(val) : QString();
            emit filenameChanged();
        } else if (propName == "media-title" && prop->format == MPV_FORMAT_STRING) {
            char *val = *static_cast<char **>(prop->data);
            m_mediaTitle = val ? QString::fromUtf8(val) : QString();
            emit mediaTitleChanged();
        } else if (propName == "ab-loop-a" && prop->format == MPV_FORMAT_DOUBLE) {
            m_loopA = *static_cast<double *>(prop->data);
            emit loopChanged();
        } else if (propName == "ab-loop-b" && prop->format == MPV_FORMAT_DOUBLE) {
            m_loopB = *static_cast<double *>(prop->data);
            emit loopChanged();
        }
        break;
    }

    case MPV_EVENT_FILE_LOADED:
        m_playing = true;
        emit playingChanged();
        emit fileLoaded();
        updateVideoParams();  // Force update on file load
        checkHdrContent();
        break;

    case MPV_EVENT_END_FILE: {
        mpv_event_end_file *eof = static_cast<mpv_event_end_file *>(event->data);
        m_playing = false;
        emit playingChanged();
        if (eof->reason == MPV_END_FILE_REASON_ERROR) {
            m_lastError = QString("Playback error: %1").arg(mpv_error_string(eof->error));
            emit errorOccurred(m_lastError);
        }
        emit endOfFile();
        break;
    }

    case MPV_EVENT_LOG_MESSAGE: {
        mpv_event_log_message *msg = static_cast<mpv_event_log_message *>(event->data);
        if (msg->log_level <= MPV_LOG_LEVEL_ERROR) {
            qWarning() << "[mpv]" << msg->prefix << ":" << msg->text;
        }
        break;
    }

    default:
        break;
    }
}

void MpvObject::updateVideoParams()
{
    if (!m_mpv) return;

    mpv_node node;
    if (mpv_get_property(m_mpv, "video-params", MPV_FORMAT_NODE, &node) >= 0) {
        QVariant params = nodeToVariant(&node);
        mpv_free_node_contents(&node);

        if (params.canConvert<QVariantMap>()) {
            QVariantMap map = params.toMap();
            m_videoWidth = map.value("dw", 0).toInt();
            m_videoHeight = map.value("dh", 0).toInt();
            m_pixelFormat = map.value("pixelformat", QString()).toString();
            m_colorPrimaries = map.value("primaries", QString()).toString();
            m_colorTransfer = map.value("gamma", QString()).toString();  // "gamma" contains transfer characteristic
            m_colorMatrix = map.value("colormatrix", QString()).toString();

            // Extract bit depth from pixel format if possible
            QString pf = m_pixelFormat.toLower();
            if (pf.contains("10le") || pf.contains("10be") || pf.contains("p010")) {
                m_bitDepth = 10;
            } else if (pf.contains("12le") || pf.contains("12be")) {
                m_bitDepth = 12;
            } else if (pf.contains("16le") || pf.contains("16be")) {
                m_bitDepth = 16;
            } else {
                m_bitDepth = 8;
            }

            emit videoParamsChanged();
            checkHdrContent();
        }
    }
}

void MpvObject::checkHdrContent()
{
    // Content is HDR if:
    // - Transfer is PQ (SMPTE ST 2084) or HLG
    // - AND primaries are BT.2020
    bool isHdr = false;

    QString transfer = m_colorTransfer.toLower();
    QString primaries = m_colorPrimaries.toLower();

    bool isPqOrHlg = transfer.contains("pq") || transfer.contains("smpte-st-2084") ||
                     transfer.contains("hlg") || transfer.contains("arib-std-b67") ||
                     transfer == "st2084" || transfer == "smpte2084";

    bool isBt2020 = primaries.contains("bt.2020") || primaries.contains("bt2020") ||
                    primaries == "2020";

    isHdr = isPqOrHlg && isBt2020;

    if (m_contentIsHdr != isHdr) {
        m_contentIsHdr = isHdr;

        // Try to get HDR metadata
        mpv_node node;
        if (mpv_get_property(m_mpv, "video-params", MPV_FORMAT_NODE, &node) >= 0) {
            QVariant params = nodeToVariant(&node);
            mpv_free_node_contents(&node);

            if (params.canConvert<QVariantMap>()) {
                QVariantMap map = params.toMap();
                m_maxCll = map.value("max-cll", 0.0).toDouble();
                m_maxFall = map.value("max-luma", 0.0).toDouble();
            }
        }

        emit hdrInfoChanged();
    }
}

void MpvObject::updateTracks()
{
    if (!m_mpv) return;

    mpv_node node;
    if (mpv_get_property(m_mpv, "track-list", MPV_FORMAT_NODE, &node) >= 0) {
        QVariant tracks = nodeToVariant(&node);
        mpv_free_node_contents(&node);

        m_audioTracks.clear();
        m_subtitleTracks.clear();

        if (tracks.canConvert<QVariantList>()) {
            QVariantList list = tracks.toList();
            for (const QVariant &track : list) {
                if (track.canConvert<QVariantMap>()) {
                    QVariantMap map = track.toMap();
                    QString type = map.value("type").toString();

                    if (type == "audio") {
                        m_audioTracks.append(map);
                    } else if (type == "sub") {
                        m_subtitleTracks.append(map);
                    }
                }
            }
        }

        emit tracksChanged();
    }
}

void MpvObject::updateChapters()
{
    if (!m_mpv) return;

    mpv_node node;
    if (mpv_get_property(m_mpv, "chapter-list", MPV_FORMAT_NODE, &node) >= 0) {
        QVariant chapters = nodeToVariant(&node);
        mpv_free_node_contents(&node);

        m_chapters.clear();

        if (chapters.canConvert<QVariantList>()) {
            m_chapters = chapters.toList();
        }

        emit chaptersChanged();
    }
}

QQuickFramebufferObject::Renderer *MpvObject::createRenderer() const
{
    window()->setPersistentSceneGraph(true);
    window()->setPersistentGraphics(true);
    return new MpvRenderer(const_cast<MpvObject *>(this));
}

void MpvObject::setMpvOption(const QString &name, const QVariant &value)
{
    if (!m_mpv) return;

    int result = 0;
    QByteArray nameUtf8 = name.toUtf8();

    if (value.typeId() == QMetaType::Bool) {
        int val = value.toBool() ? 1 : 0;
        result = mpv_set_option(m_mpv, nameUtf8.constData(), MPV_FORMAT_FLAG, &val);
    } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
        int64_t val = value.toLongLong();
        result = mpv_set_option(m_mpv, nameUtf8.constData(), MPV_FORMAT_INT64, &val);
    } else if (value.typeId() == QMetaType::Double) {
        double val = value.toDouble();
        result = mpv_set_option(m_mpv, nameUtf8.constData(), MPV_FORMAT_DOUBLE, &val);
    } else {
        QByteArray valUtf8 = value.toString().toUtf8();
        result = mpv_set_option_string(m_mpv, nameUtf8.constData(), valUtf8.constData());
    }

    if (result < 0) {
        qWarning() << "Failed to set mpv option" << name << ":" << mpv_error_string(result);
    }
}

void MpvObject::setMpvProperty(const QString &name, const QVariant &value)
{
    if (!m_mpv) return;

    int result = 0;
    QByteArray nameUtf8 = name.toUtf8();

    if (value.typeId() == QMetaType::Bool) {
        int val = value.toBool() ? 1 : 0;
        result = mpv_set_property(m_mpv, nameUtf8.constData(), MPV_FORMAT_FLAG, &val);
    } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
        int64_t val = value.toLongLong();
        result = mpv_set_property(m_mpv, nameUtf8.constData(), MPV_FORMAT_INT64, &val);
    } else if (value.typeId() == QMetaType::Double) {
        double val = value.toDouble();
        result = mpv_set_property(m_mpv, nameUtf8.constData(), MPV_FORMAT_DOUBLE, &val);
    } else {
        QByteArray valUtf8 = value.toString().toUtf8();
        result = mpv_set_property_string(m_mpv, nameUtf8.constData(), valUtf8.constData());
    }

    if (result < 0) {
        qWarning() << "Failed to set mpv property" << name << ":" << mpv_error_string(result);
    }
}

QVariant MpvObject::getMpvPropertyVariant(const QString &name) const
{
    if (!m_mpv) return QVariant();

    mpv_node node;
    if (mpv_get_property(m_mpv, name.toUtf8().constData(), MPV_FORMAT_NODE, &node) >= 0) {
        QVariant result = nodeToVariant(&node);
        mpv_free_node_contents(&node);
        return result;
    }
    return QVariant();
}

QVariant MpvObject::getMpvProperty(const QString &name) const
{
    return getMpvPropertyVariant(name);
}

QString MpvObject::getMpvVersion() const
{
    if (!m_mpv) return QString();

    const char *version = mpv_get_property_string(m_mpv, "mpv-version");
    if (version) {
        QString result = QString::fromUtf8(version);
        mpv_free((void *)version);
        return result;
    }
    return QString();
}

// Playback control implementations
void MpvObject::loadFile(const QString &path)
{
    if (!m_mpv) return;

    QByteArray pathUtf8 = path.toUtf8();
    const char *args[] = {"loadfile", pathUtf8.constData(), nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::play()
{
    setMpvProperty("pause", false);
}

void MpvObject::pause()
{
    setMpvProperty("pause", true);
}

void MpvObject::stop()
{
    if (!m_mpv) return;
    const char *args[] = {"stop", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::togglePause()
{
    if (!m_mpv) return;
    const char *args[] = {"cycle", "pause", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::seek(double seconds)
{
    if (!m_mpv) return;
    QByteArray secStr = QByteArray::number(seconds);
    const char *args[] = {"seek", secStr.constData(), "relative", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::seekAbsolute(double seconds)
{
    if (!m_mpv) return;
    QByteArray secStr = QByteArray::number(seconds);
    const char *args[] = {"seek", secStr.constData(), "absolute", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::seekPercent(double percent)
{
    if (!m_mpv) return;
    QByteArray pctStr = QByteArray::number(percent);
    const char *args[] = {"seek", pctStr.constData(), "absolute-percent", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::setVolume(int vol)
{
    setMpvProperty("volume", qBound(0, vol, 150));
}

void MpvObject::setMuted(bool muted)
{
    setMpvProperty("mute", muted);
}

void MpvObject::setSpeed(double speed)
{
    setMpvProperty("speed", qBound(0.25, speed, 4.0));
}

void MpvObject::setAudioTrack(int id)
{
    setMpvProperty("aid", id);
}

void MpvObject::setSubtitleTrack(int id)
{
    setMpvProperty("sid", id);
}

void MpvObject::loadSubtitleFile(const QString &path)
{
    if (!m_mpv) return;
    QByteArray pathUtf8 = path.toUtf8();
    const char *args[] = {"sub-add", pathUtf8.constData(), "select", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::setChapter(int index)
{
    setMpvProperty("chapter", index);
}

void MpvObject::nextChapter()
{
    if (!m_mpv) return;
    const char *args[] = {"add", "chapter", "1", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::previousChapter()
{
    if (!m_mpv) return;
    const char *args[] = {"add", "chapter", "-1", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::frameStep()
{
    if (!m_mpv) return;
    const char *args[] = {"frame-step", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::frameBackStep()
{
    if (!m_mpv) return;
    const char *args[] = {"frame-back-step", nullptr};
    mpv_command_async(m_mpv, 0, args);
}

void MpvObject::setLoopA()
{
    setMpvProperty("ab-loop-a", m_position);
}

void MpvObject::setLoopB()
{
    setMpvProperty("ab-loop-b", m_position);
}

void MpvObject::clearLoop()
{
    setMpvProperty("ab-loop-a", "no");
    setMpvProperty("ab-loop-b", "no");
}

void MpvObject::setHdrMode(const QString &mode)
{
    SettingsManager::instance()->setHdrMode(mode);

    // Apply at runtime
    if (mode == "passthrough") {
        setMpvProperty("tone-mapping", "clip");
        setMpvProperty("hdr-compute-peak", "no");
        setMpvProperty("target-colorspace-hint", "yes");
    } else if (mode == "tonemap") {
        setMpvProperty("tone-mapping", "hable");
        setMpvProperty("hdr-compute-peak", "yes");
        setMpvProperty("target-colorspace-hint", "no");
    } else {
        setMpvProperty("tone-mapping", "auto");
        setMpvProperty("hdr-compute-peak", "auto");
        setMpvProperty("target-colorspace-hint", "yes");
    }
}

void MpvObject::setHwdecMode(const QString &mode)
{
    SettingsManager::instance()->setHwdecMode(mode);

    if (mode == "on") {
        setMpvProperty("hwdec", "auto-safe");
    } else if (mode == "off") {
        setMpvProperty("hwdec", "no");
    } else {
        setMpvProperty("hwdec", "auto-safe");
    }
}

void MpvObject::setRendererMode(const QString &mode)
{
    SettingsManager::instance()->setRendererMode(mode);
    // Note: gpu-api typically requires restart to take effect
}

void MpvObject::onUpdate(void *ctx)
{
    MpvObject *self = static_cast<MpvObject *>(ctx);
    QMetaObject::invokeMethod(self, "update", Qt::QueuedConnection);
}
