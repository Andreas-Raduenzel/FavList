#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QWindow>
#include <QSharedMemory>

// NEU für Tray:
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "FavoriteBackend.h"
#include "autostartmanager.h"

#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 🔹 Prüfen, ob wir mit --autostart gestartet wurden
    bool startedFromAutostart = false;
    QStringList args = app.arguments();
    if (args.contains("--autostart")) {
        startedFromAutostart = true;
    }

    // 🔹 Single-Instance
    QSharedMemory sharedMemory("FavListSingleInstanceKey");
    if (!sharedMemory.create(1)) {
        qDebug() << "FavList läuft bereits. Beende zweite Instanz.";
        return 0;
    }

    QCoreApplication::setOrganizationName("crumbTechFavApp");
    QCoreApplication::setApplicationName("FavList");
    QGuiApplication::setDesktopFileName("favlist");

    QIcon appIcon(":/resources/icons/appicon.png");
    app.setWindowIcon(appIcon);

    QStringList fallbackThemes = {"breeze", "hicolor", "Adwaita", "Mint-Y"};
    bool themeFound = false;

    for (const QString &theme : fallbackThemes) {
        QIcon::setThemeName(theme);
        if (!QIcon::fromTheme("folder").isNull()) {
            qDebug() << "Icon-Theme verwendet:" << theme;
            themeFound = true;
            break;
        }
    }

    if (!themeFound) {
        qDebug() << "Kein passendes Icon-Theme gefunden. Icons evtl. nicht sichtbar.";
    }

    // 🔹 Prüfen, ob überhaupt ein System-Tray verfügbar ist
    bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    /*trayAvailable = false;   // <<< TEST-MODUS: so tun, als gäbe es keinen Tray*/

    if (!trayAvailable) {
        qWarning() << "System Tray nicht verfügbar!";
    }

    // 🔹 Verhalten beim letzten Fenster:
    //    - mit Tray: App weiterlaufen lassen
    //    - ohne Tray: ganz normal beenden
    if (trayAvailable) {
        app.setQuitOnLastWindowClosed(false);
    } else {
        app.setQuitOnLastWindowClosed(true);
    }

    QQmlApplicationEngine engine;

    // 🔹 C++-Objekte anlegen
    AutostartManager autostartManager;
    FavoriteBackend backend;

    // 🔹 Kontext-Properties für QML
    engine.rootContext()->setContextProperty("autostartManager", &autostartManager);
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.rootContext()->setContextProperty("trayAvailable", trayAvailable);
    engine.rootContext()->setContextProperty("startedFromAutostart", startedFromAutostart);

    QString qmlPath = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/../qml/main.qml"
    );

    if (!QFile::exists(qmlPath)) {
        qmlPath = "/usr/share/favoriten/qml/main.qml";
    }

    engine.load(QUrl::fromLocalFile(qmlPath));

    if (engine.rootObjects().isEmpty())
        return -1;

    QObject *topLevel = engine.rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(topLevel);
    if (window) {
        window->setIcon(appIcon);
    }

    // 🔹 Nur Tray-Icon und Menü anlegen, wenn ein System-Tray verfügbar ist
    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;

    if (trayAvailable) {

        trayIcon = new QSystemTrayIcon(&app);
        trayIcon->setIcon(appIcon);
        trayIcon->setToolTip("Favorite");

        trayMenu = new QMenu();
        QAction *showAction = new QAction("Öffnen", trayMenu);
        QAction *settingsAction = new QAction("Einstellungen...", trayMenu);
        QAction *quitAction = new QAction("Beenden", trayMenu);

        // Reihenfolge im Menü
        trayMenu->addAction(showAction);
        trayMenu->addAction(settingsAction);
        trayMenu->addSeparator();
        trayMenu->addAction(quitAction);

        trayIcon->setContextMenu(trayMenu);
        trayIcon->show();

        // ✅ NUR Tray-Icon anzeigen, wenn:
        //    - aus Autostart gestartet
        //    - und "nur Tray" aktiviert
        if (startedFromAutostart
                && autostartManager.startOnlyTray()
                && window) {
            window->hide();
        }

        // Tray-Icon Klick: Fenster ein-/ausblenden
        QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                         &app, [window](QSystemTrayIcon::ActivationReason reason) {
            if (!window)
                return;

            if (reason == QSystemTrayIcon::Trigger ||
                reason == QSystemTrayIcon::DoubleClick) {

                if (window->isVisible()) {
                    window->hide();
                } else {
                    window->show();
                    window->raise();
                    window->requestActivate();
                }
            }
        });

        // "Öffnen" im Menü
        QObject::connect(showAction, &QAction::triggered, [window]() {
            if (!window)
                return;
            window->show();
            window->raise();
            window->requestActivate();
        });

        // "Einstellungen..." im Menü → nur Settings-Fenster öffnen
        QObject::connect(settingsAction, &QAction::triggered, [&engine]() {
            if (engine.rootObjects().isEmpty())
                return;

            QObject *rootObject = engine.rootObjects().first();
            QMetaObject::invokeMethod(rootObject, "openSettings",
                                      Qt::QueuedConnection);
        });

        // "Beenden" im Menü
        QObject::connect(quitAction, &QAction::triggered,
                         &app, &QCoreApplication::quit);
    }

    return app.exec();
}
