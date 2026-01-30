#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QtQml>
#include <clocale>

#include "mpvobject.h"
#include "playercontroller.h"
#include "settingsmanager.h"
#include "hdrdiagnostics.h"
#include "recentfilesmodel.h"
#include "trackmodel.h"
#include "chaptermodel.h"

int main(int argc, char *argv[])
{
    // Set C locale for mpv (required by libmpv)
    // Must be called before any Qt or mpv initialization
    std::setlocale(LC_NUMERIC, "C");

    // Required for proper Wayland support
    qputenv("QT_QPA_PLATFORM", "wayland;xcb");

    // Enable high DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // Ensure C locale remains set after Qt initialization
    std::setlocale(LC_NUMERIC, "C");

    app.setApplicationName("Absokino");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Absokino");
    app.setOrganizationDomain("absokino.local");
    app.setWindowIcon(QIcon::fromTheme("video-player"));

    // Set Breeze Dark style for Kirigami
    QQuickStyle::setStyle("org.kde.desktop");

    // Register QML types
    qmlRegisterType<MpvObject>("Absokino.Mpv", 1, 0, "MpvObject");
    qmlRegisterType<TrackModel>("Absokino.Models", 1, 0, "TrackModel");
    qmlRegisterType<ChapterModel>("Absokino.Models", 1, 0, "ChapterModel");
    qmlRegisterSingletonType<PlayerController>("Absokino.Player", 1, 0, "PlayerController",
        [](QQmlEngine *engine, QJSEngine *) -> QObject * {
            Q_UNUSED(engine)
            return PlayerController::instance();
        });
    qmlRegisterSingletonType<SettingsManager>("Absokino.Settings", 1, 0, "Settings",
        [](QQmlEngine *engine, QJSEngine *) -> QObject * {
            Q_UNUSED(engine)
            return SettingsManager::instance();
        });
    qmlRegisterSingletonType<HdrDiagnostics>("Absokino.Diagnostics", 1, 0, "HdrDiagnostics",
        [](QQmlEngine *engine, QJSEngine *) -> QObject * {
            Q_UNUSED(engine)
            return HdrDiagnostics::instance();
        });
    qmlRegisterSingletonType<RecentFilesModel>("Absokino.Models", 1, 0, "RecentFiles",
        [](QQmlEngine *engine, QJSEngine *) -> QObject * {
            Q_UNUSED(engine)
            return RecentFilesModel::instance();
        });

    QQmlApplicationEngine engine;

    // Handle file argument
    QStringList args = app.arguments();
    if (args.size() > 1) {
        QString filePath = args.at(1);
        if (QFile::exists(filePath)) {
            PlayerController::instance()->setInitialFile(filePath);
        }
    }

    const QUrl url(QStringLiteral("qrc:/Absokino/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
