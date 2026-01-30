#include "hdrdiagnostics.h"
#include "mpvobject.h"
#include "settingsmanager.h"
#include "playercontroller.h"

#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegularExpression>

HdrDiagnostics *HdrDiagnostics::s_instance = nullptr;

HdrDiagnostics *HdrDiagnostics::instance()
{
    if (!s_instance) {
        s_instance = new HdrDiagnostics();
    }
    return s_instance;
}

HdrDiagnostics::HdrDiagnostics(QObject *parent)
    : QObject(parent)
{
}

QString HdrDiagnostics::generateReport()
{
    QStringList report;

    report << "=== Absokino HDR/Output Diagnostics ===";
    report << "";

    // ===== Section 1: MPV Version Info =====
    {
        QStringList lines;
        if (m_mpvObject) {
            lines << QString("mpv version: %1").arg(m_mpvObject->getMpvVersion());
            lines << QString("libmpv client API: %1.%2")
                .arg(MPV_CLIENT_API_VERSION >> 16)
                .arg(MPV_CLIENT_API_VERSION & 0xFFFF);
        } else {
            lines << "mpv: Not initialized";
        }
        report << formatSection("MPV Information", lines);
    }

    // ===== Section 2: Renderer/Backend =====
    {
        QStringList lines;
        if (m_mpvObject) {
            QString vo = m_mpvObject->voBackend();
            QString gpuApi = m_mpvObject->gpuApi();
            QString hwdec = m_mpvObject->hwdecCurrent();

            lines << QString("Video output (vo): %1").arg(vo.isEmpty() ? "libmpv" : vo);
            lines << QString("GPU API: %1").arg(gpuApi.isEmpty() ? "(embedded OpenGL via Qt)" : gpuApi);
            lines << QString("Hardware decoding: %1").arg(hwdec.isEmpty() ? "none/software" : hwdec);
        }
        report << formatSection("Renderer/Backend", lines);
    }

    // ===== Section 3: Content Color Information =====
    {
        QStringList lines;
        if (m_mpvObject && m_mpvObject->videoWidth() > 0) {
            lines << QString("Resolution: %1x%2").arg(m_mpvObject->videoWidth()).arg(m_mpvObject->videoHeight());
            lines << QString("FPS: %1").arg(m_mpvObject->fps(), 0, 'f', 3);
            lines << QString("Pixel format: %1").arg(m_mpvObject->pixelFormat());
            lines << QString("Bit depth: %1-bit").arg(m_mpvObject->bitDepth());
            lines << QString("Color primaries: %1").arg(m_mpvObject->colorPrimaries());
            lines << QString("Transfer (gamma): %1").arg(m_mpvObject->colorTransfer());
            lines << QString("Color matrix: %1").arg(m_mpvObject->colorMatrix());
            lines << "";

            bool isHdr = m_mpvObject->contentIsHdr();
            lines << QString("*** Content is HDR: %1 ***").arg(isHdr ? "YES" : "NO");

            if (isHdr) {
                double maxCll = m_mpvObject->maxCll();
                double maxFall = m_mpvObject->maxFall();
                if (maxCll > 0) {
                    lines << QString("MaxCLL: %1 nits").arg(maxCll);
                }
                if (maxFall > 0) {
                    lines << QString("MaxFALL: %1 nits").arg(maxFall);
                }
            }
        } else {
            lines << "No video loaded or video params unavailable";
        }
        report << formatSection("Content Color Information", lines);
    }

    // ===== Section 4: MPV HDR Configuration =====
    {
        QStringList lines;
        if (m_mpvObject) {
            QVariant targetTrc = m_mpvObject->getMpvProperty("target-trc");
            QVariant targetPrim = m_mpvObject->getMpvProperty("target-prim");
            QVariant toneMapping = m_mpvObject->getMpvProperty("tone-mapping");
            QVariant hdrComputePeak = m_mpvObject->getMpvProperty("hdr-compute-peak");
            QVariant targetColorspaceHint = m_mpvObject->getMpvProperty("target-colorspace-hint");

            lines << QString("target-trc: %1").arg(targetTrc.toString());
            lines << QString("target-prim: %1").arg(targetPrim.toString());
            lines << QString("tone-mapping: %1").arg(toneMapping.toString());
            lines << QString("hdr-compute-peak: %1").arg(hdrComputePeak.toString());
            lines << QString("target-colorspace-hint: %1").arg(targetColorspaceHint.toString());
        }

        SettingsManager *settings = SettingsManager::instance();
        lines << "";
        lines << QString("App HDR mode setting: %1").arg(settings->hdrMode());
        lines << QString("App hwdec mode setting: %1").arg(settings->hwdecMode());
        lines << QString("App renderer mode setting: %1").arg(settings->rendererMode());

        report << formatSection("MPV HDR Configuration", lines);
    }

    // ===== Section 5: Output Mode Assessment =====
    {
        QStringList lines;
        QString outputMode = determinOutputMode();
        lines << QString("Assessed output mode: %1").arg(outputMode);

        if (outputMode == "Passthrough") {
            lines << "mpv is configured to pass HDR content without tone-mapping.";
            lines << "Whether the display actually receives HDR depends on the display/compositor.";
        } else if (outputMode == "Tone-mapped") {
            lines << "mpv is tone-mapping HDR content to SDR.";
        } else {
            lines << "Content is SDR, no HDR processing needed.";
        }

        report << formatSection("Output Mode Assessment", lines);
    }

    // ===== Section 6: Display HDR State =====
    {
        QStringList lines;
        QString displayHdr = checkDisplayHdrCapability();

        lines << QString("Display HDR state: %1").arg(displayHdr);
        lines << "";

        if (displayHdr == "Unknown") {
            lines << "IMPORTANT: We cannot reliably determine if your display is in HDR mode.";
            lines << "On Linux/Wayland, detecting actual display HDR state is extremely difficult.";
            lines << "This does NOT mean HDR isn't working - we simply cannot confirm it.";
            lines << "";
            lines << "Heuristics checked:";
            lines << QString("  - KDE HDR setting (via D-Bus): %1").arg(checkKdeHdrState());
            lines << QString("  - DRM HDR metadata: %1").arg(checkDrmHdrState());
        } else if (displayHdr == "Confirmed") {
            lines << "Display appears to be in HDR mode based on available indicators.";
        } else {
            lines << "Could not check display HDR capability.";
        }

        report << formatSection("Display HDR State", lines);
    }

    // ===== Section 7: Suggestions =====
    {
        QStringList suggestions = generateSuggestions();
        if (!suggestions.isEmpty()) {
            report << formatSection("Actionable Suggestions", suggestions);
        }
    }

    // ===== Disclaimer =====
    report << "=== Disclaimer ===";
    report << "This diagnostic tool provides best-effort information.";
    report << "Display HDR state shown as 'Unknown' is normal on Linux.";
    report << "Visual confirmation (e.g., HDR indicator on display) is the most reliable test.";
    report << "";

    m_lastReport = report.join("\n");
    emit reportGenerated();
    return m_lastReport;
}

QString HdrDiagnostics::formatSection(const QString &title, const QStringList &lines)
{
    QString result;
    result += QString("\n--- %1 ---\n").arg(title);
    for (const QString &line : lines) {
        result += line + "\n";
    }
    return result;
}

QString HdrDiagnostics::determinOutputMode()
{
    if (!m_mpvObject) {
        return "Unknown";
    }

    bool contentIsHdr = m_mpvObject->contentIsHdr();
    if (!contentIsHdr) {
        return "SDR";
    }

    // Check if tone-mapping is being applied
    QVariant toneMapping = m_mpvObject->getMpvProperty("tone-mapping");
    QString tm = toneMapping.toString().toLower();

    // "clip" or "no" means minimal/no tone-mapping (passthrough intent)
    if (tm == "clip" || tm == "no" || tm.isEmpty()) {
        return "Passthrough";
    }

    // If tone-mapping is set to an algorithm, it's being tone-mapped
    if (tm == "hable" || tm == "mobius" || tm == "reinhard" ||
        tm == "bt.2390" || tm == "gamma" || tm == "linear") {
        return "Tone-mapped";
    }

    // "auto" - depends on target-trc and other factors
    if (tm == "auto") {
        QVariant hdrComputePeak = m_mpvObject->getMpvProperty("hdr-compute-peak");
        if (hdrComputePeak.toString() == "yes" || hdrComputePeak.toBool()) {
            return "Tone-mapped";
        }
        return "Passthrough (auto)";
    }

    return "Unknown";
}

QVariantMap HdrDiagnostics::getCompactStatus()
{
    QVariantMap status;

    if (!m_mpvObject) {
        status["contentHdr"] = "N/A";
        status["outputMode"] = "N/A";
        status["displayHdr"] = "N/A";
        return status;
    }

    // Content HDR
    bool contentHdr = m_mpvObject->contentIsHdr();
    status["contentHdr"] = contentHdr ? "Yes" : "No";

    // Output mode
    if (contentHdr) {
        status["outputMode"] = determinOutputMode();
    } else {
        status["outputMode"] = "SDR";
    }

    // Display HDR - always be honest about uncertainty
    QString displayState = checkDisplayHdrCapability();
    status["displayHdr"] = displayState;

    return status;
}

QString HdrDiagnostics::checkDisplayHdrCapability()
{
    // Try multiple methods, but be honest about uncertainty

    // Method 1: Check KDE HDR setting via D-Bus
    QString kdeState = checkKdeHdrState();
    if (kdeState == "enabled") {
        // KDE says HDR is enabled, but this is still just a setting
        // It doesn't guarantee the display is actually receiving HDR
        return "Unknown";  // We still can't confirm actual display state
    }

    // Method 2: Check DRM/KMS HDR metadata
    QString drmState = checkDrmHdrState();
    if (drmState == "active") {
        // DRM reports HDR metadata is being sent
        // This is a stronger indicator but still not 100% confirmation
        return "Unknown";  // Be conservative - we can't truly confirm
    }

    // We cannot reliably determine display HDR state on Linux
    return "Unknown";
}

QString HdrDiagnostics::checkKdeHdrState()
{
    // Try to query KDE's HDR setting via D-Bus
    // Note: This checks the SETTING, not whether HDR is actually working

    QDBusInterface iface("org.kde.KWin",
                         "/org/kde/KWin",
                         "org.kde.KWin",
                         QDBusConnection::sessionBus());

    if (!iface.isValid()) {
        return "unavailable";
    }

    // KDE Plasma 6 might expose HDR info differently
    // This is a best-effort check
    QDBusReply<QVariantMap> reply = iface.call("supportInformation");
    if (reply.isValid()) {
        QString info = reply.value().value("supportInformation").toString();
        if (info.contains("HDR: enabled", Qt::CaseInsensitive)) {
            return "enabled";
        } else if (info.contains("HDR: disabled", Qt::CaseInsensitive)) {
            return "disabled";
        }
    }

    return "unknown";
}

QString HdrDiagnostics::checkDrmHdrState()
{
    // Check DRM connectors for HDR metadata
    // This is a heuristic - presence of HDR metadata doesn't guarantee
    // the display is actually showing HDR content correctly

    QDir drmDir("/sys/class/drm");
    if (!drmDir.exists()) {
        return "unavailable";
    }

    QStringList cards = drmDir.entryList(QStringList() << "card*", QDir::Dirs);
    for (const QString &card : cards) {
        QString connectorPath = drmDir.filePath(card);
        QDir connectorDir(connectorPath);

        // Look for HDR-related files
        QStringList hdrFiles = connectorDir.entryList(
            QStringList() << "*hdr*" << "*HDR*", QDir::Files);

        if (!hdrFiles.isEmpty()) {
            // Found HDR-related sysfs entries
            // Check if HDR output metadata is present
            for (const QString &hdrFile : hdrFiles) {
                QFile f(connectorDir.filePath(hdrFile));
                if (f.open(QIODevice::ReadOnly)) {
                    QString content = QString::fromUtf8(f.readAll()).trimmed();
                    f.close();

                    if (!content.isEmpty() && content != "0") {
                        return "active";
                    }
                }
            }
        }
    }

    return "not_detected";
}

QString HdrDiagnostics::checkWaylandHdrState()
{
    // Wayland doesn't provide a standard way to query HDR state
    // This would require compositor-specific protocols

    // Check if running on Wayland
    QString sessionType = qEnvironmentVariable("XDG_SESSION_TYPE");
    if (sessionType.toLower() != "wayland") {
        return "not_wayland";
    }

    // Check KDE-specific method
    return checkKdeHdrState();
}

QStringList HdrDiagnostics::generateSuggestions()
{
    QStringList suggestions;

    if (!m_mpvObject) {
        suggestions << "- Initialize video playback to get diagnostics";
        return suggestions;
    }

    bool contentIsHdr = m_mpvObject->contentIsHdr();
    QString outputMode = determinOutputMode();
    QString hwdec = m_mpvObject->hwdecCurrent();

    // HDR content suggestions
    if (contentIsHdr) {
        if (outputMode != "Passthrough" && outputMode != "Passthrough (auto)") {
            suggestions << "- For HDR passthrough, set HDR mode to 'Passthrough preferred' in Settings";
            suggestions << "- Ensure KDE Display Settings has HDR enabled";
        }

        // Check if fullscreen (passthrough often works better in fullscreen)
        if (!PlayerController::instance()->isFullscreen()) {
            suggestions << "- Try fullscreen mode - HDR passthrough may work better";
        }

        // Hardware decoding suggestions for HDR
        if (hwdec.isEmpty()) {
            suggestions << "- Enable hardware decoding for better HDR performance";
        }
    }

    // General suggestions
    if (hwdec.isEmpty() && m_mpvObject->videoHeight() >= 2160) {
        suggestions << "- Consider enabling hardware decoding for 4K content";
    }

    // Check display HDR
    QString kdeState = checkKdeHdrState();
    if (contentIsHdr && kdeState != "enabled") {
        suggestions << "- Enable HDR in KDE System Settings > Display > HDR";
    }

    if (suggestions.isEmpty()) {
        suggestions << "- Current configuration appears optimal for this content";
    }

    return suggestions;
}
