#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QWindow>
#include <QSharedMemory>

// NEU:
#include <QScreen>
#include <QCursor>

// NEU für Tray:
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "FavoriteBackend.h"
#include "autostartmanager.h"

#include <QFile>

// NEU: Hilfsfunktion, um das Fenster über der unteren Leiste zu platzieren
static void showPopupOverBottomPanel(QWindow *window)
{
    if (!window)
        return;

    int popupWidth  = window->width()  > 0 ? window->width()  : 250;
    int popupHeight = window->height() > 0 ? window->height() : 400;

    QPoint clickPos = QCursor::pos();

    QScreen *screen = QGuiApplication::screenAt(clickPos);
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    // kompletter Screen + der nutzbare Bereich ohne Panel
    QRect screenGeo = screen->geometry();
    QRect availGeo  = screen->availableGeometry();

    const int sideMargin    = 5;   // links/rechts
    const int gapAbovePanel = 60;  // "5 mm Abstand" – kannst du nach Gefühl anpassen

    // X: an der Maus zentrieren, aber im sichtbaren Bereich bleiben
    int x = clickPos.x() - popupWidth / 2;
    if (x < availGeo.left() + sideMargin)
        x = availGeo.left() + sideMargin;
    if (x + popupWidth > availGeo.right() - sideMargin)
        x = availGeo.right() - sideMargin - popupWidth;

    // Y: an der Oberkante der Taskleiste ausrichten (availGeo.bottom())
    //    und ein Stück nach oben schieben, damit es "schwebt"
    int y = availGeo.bottom() - popupHeight - gapAbovePanel;

    window->resize(popupWidth, popupHeight);
    window->setX(x);
    window->setY(y);

    window->show();
    window->raise();
    window->requestActivate();
}


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

    // 🔹 Icon-Theme-Verhalten:
    QString currentTheme = QIcon::themeName();
    if (currentTheme.isEmpty()) {
        QIcon::setThemeName("hicolor");
        qDebug() << "Kein System-Icon-Theme gesetzt. Fallback auf 'hicolor'.";
    } else {
        qDebug() << "System-Icon-Theme wird verwendet:" << currentTheme;
    }

    // 🔹 Prüfen, ob überhaupt ein System-Tray verfügbar ist
    bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    /*trayAvailable = false;   // <<< TEST-MODUS: so tun, als gäbe es keinen Tray*/

    if (!trayAvailable) {
        qWarning() << "System Tray nicht verfügbar!";
    }

    // 🔹 Verhalten beim letzten Fenster:
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

        // NEU: Standardverhalten beim Start
        // - Wenn kein Tray vorhanden -> Fenster normal zeigen
        // - Wenn aus Autostart und "nur Tray" -> NICHT zeigen
        // - Sonst (manuell gestartet) -> Fenster zeigen
        if (!trayAvailable
            || !startedFromAutostart
            || !autostartManager.startOnlyTray()) {
            window->show();
        }
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

        // Tray-Icon Klick: Fenster ein-/ausblenden, mit Popup über der Leiste
      QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                 &app, [window](QSystemTrayIcon::ActivationReason reason) {
    if (!window)
        return;

    if (reason == QSystemTrayIcon::Trigger ||
        reason == QSystemTrayIcon::DoubleClick) {

        Qt::WindowStates state = window->windowState();

        // 🔹 Fall 1: Fenster ist minimiert → Zustand zurücksetzen + schön platzieren
        if (state.testFlag(Qt::WindowMinimized)) {
            window->setWindowState(Qt::WindowNoState);
            showPopupOverBottomPanel(window);
            return;
        }

        // 🔹 Fall 2: Fenster ist unsichtbar → anzeigen
        if (!window->isVisible()) {
            showPopupOverBottomPanel(window);
            return;
        }

        // 🔹 Fall 3: Fenster ist sichtbar (und nicht minimiert) → verstecken
        window->hide();
    }
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
