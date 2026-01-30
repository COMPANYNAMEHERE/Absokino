#ifndef HDRDIAGNOSTICS_H
#define HDRDIAGNOSTICS_H

#include <QObject>
#include <QString>
#include <QVariantMap>

class MpvObject;

/**
 * @brief HdrDiagnostics - Generates detailed HDR and output diagnostics
 *
 * This class collects information from mpv, the system, and attempts
 * to determine HDR state. It is careful NOT to make misleading claims
 * about display HDR state when it cannot be reliably determined.
 */
class HdrDiagnostics : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastReport READ lastReport NOTIFY reportGenerated)

public:
    static HdrDiagnostics *instance();

    void setMpvObject(MpvObject *mpv) { m_mpvObject = mpv; }
    MpvObject *mpvObject() const { return m_mpvObject; }

    QString lastReport() const { return m_lastReport; }

public slots:
    /**
     * @brief generateReport - Generate comprehensive HDR/output diagnostics
     * @return Full diagnostic report as formatted text
     */
    QString generateReport();

    /**
     * @brief getCompactStatus - Get compact status for status bar
     * @return Map with keys: contentHdr, outputMode, displayHdr
     */
    QVariantMap getCompactStatus();

    /**
     * @brief checkDisplayHdrCapability - Attempt to detect display HDR state
     * @return "confirmed", "unknown", or "unavailable"
     *
     * NOTE: On Linux/Wayland, reliably detecting if the display is in HDR mode
     * is extremely difficult. This method returns "unknown" unless we have
     * a reliable mechanism (which is rare).
     */
    QString checkDisplayHdrCapability();

signals:
    void reportGenerated();

private:
    explicit HdrDiagnostics(QObject *parent = nullptr);
    ~HdrDiagnostics() override = default;

    QString formatSection(const QString &title, const QStringList &lines);
    QString determinOutputMode();
    QStringList generateSuggestions();

    // Platform-specific detection methods
    QString checkWaylandHdrState();
    QString checkKdeHdrState();
    QString checkDrmHdrState();

    static HdrDiagnostics *s_instance;
    MpvObject *m_mpvObject = nullptr;
    QString m_lastReport;
};

#endif // HDRDIAGNOSTICS_H
