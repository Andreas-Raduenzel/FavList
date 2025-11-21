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
#include "autostartmanager.h"   // ✅ NEU

#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSharedMemory sharedMemory("FavListSingleInstanceKey");
    if (!sharedMemory.create(1)) {
        qDebug() << "FavList läuft bereits. Beende zweite Instanz.";
        return 0;
    }

    app.setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("crumbTechFavApp");
    QCoreApplication::setApplicationName("FavList");
    QGuiApplication::setDesktopFileName("favlist");

    QIcon appIcon(":/resources/icons/appicon.svg");
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

    QQmlApplicationEngine engine;

    // ✅ HIER: Objekte anlegen
    AutostartManager autostartManager;       // <--- NEU
    FavoriteBackend backend;

    // ✅ Reihenfolge: erst Objekte, dann ins QML geben
    engine.rootContext()->setContextProperty("autostartManager", &autostartManager);
    engine.rootContext()->setContextProperty("backend", &backend);

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

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System Tray nicht verfügbar!";
    }

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&app);
    trayIcon->setIcon(appIcon);
    trayIcon->setToolTip("Favorite");

    QMenu *trayMenu = new QMenu();
    QAction *showAction = new QAction("Öffnen", trayMenu);
    QAction *settingsAction = new QAction("Einstellungen...", trayMenu);  // ✅ neu
    QAction *quitAction = new QAction("Beenden", trayMenu);

    // Reihenfolge im Menü
    trayMenu->addAction(showAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

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


    return app.exec();
}
