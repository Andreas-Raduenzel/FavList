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

#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //AppSettings appSettings;

    // ❗ Nur eine Instanz erlauben
    QSharedMemory sharedMemory("FavListSingleInstanceKey");
    if (!sharedMemory.create(1)) {
        qDebug() << "FavList läuft bereits. Beende zweite Instanz.";
        return 0;   // Sofort beenden, kein zweites Tray-Icon
    }

    // Wichtig: App soll weiterlaufen, auch wenn kein Fenster mehr offen ist
    app.setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("crumbTechFavApp");
    QCoreApplication::setApplicationName("FavList");

    // Desktop-Dateiname für Integration unter Linux
    QGuiApplication::setDesktopFileName("favlist");  // ohne Pfad

    // App-Icon laden
    QIcon appIcon(":/resources/icons/appicon.svg");
    app.setWindowIcon(appIcon);


    // Icon-Theme-Fallbacks
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

    // QML Engine starten
    QQmlApplicationEngine engine;

    // Backend-Objekt für QML verfügbar machen
    FavoriteBackend backend;
    engine.rootContext()->setContextProperty("backend", &backend);

    // QML-Datei laden: erst Dev-Pfad, dann System-Pfad als Fallback
    QString qmlPath = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/../qml/main.qml"
    );

    if (!QFile::exists(qmlPath)) {
        // Fallback für installierte Pakete
        qmlPath = "/usr/share/favoriten/qml/main.qml";
    }

    engine.load(QUrl::fromLocalFile(qmlPath));

    if (engine.rootObjects().isEmpty())
        return -1;


    // Hauptfenster ermitteln
    QObject *topLevel = engine.rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(topLevel);
    if (window) {
        window->setIcon(appIcon);
    }

    // --- System-Tray-Icon einrichten ---
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System Tray nicht verfügbar!";
    }

    // Tray-Icon erstellen
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&app);
    trayIcon->setIcon(appIcon);
    trayIcon->setToolTip("Favorite");

    // Kontextmenü für Tray
    QMenu *trayMenu = new QMenu();
    QAction *showAction = new QAction("Öffnen", trayMenu);
    QAction *quitAction = new QAction("Beenden", trayMenu);

    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Klick auf Tray-Icon: Fenster ein-/ausblenden
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

    // "Beenden" im Menü
    QObject::connect(quitAction, &QAction::triggered,
                     &app, &QCoreApplication::quit);

    return app.exec();
}
