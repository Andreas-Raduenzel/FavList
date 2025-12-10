#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QWindow>
#include <QSharedMemory>

#include <QScreen>
#include <QCursor>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "FavoriteBackend.h"
#include "autostartmanager.h"

#include <QFile>

// Hilfsfunktion, um das Fenster automatisch an Leiste zu platzieren
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

    QRect screenGeo = screen->geometry();
    QRect availGeo  = screen->availableGeometry();

    const int sideMargin = 5;   // links/rechts

    // X: an der Maus zentrieren, aber im sichtbaren Bereich bleiben
    int x = clickPos.x() - popupWidth / 2;
    if (x < availGeo.left() + sideMargin)
        x = availGeo.left() + sideMargin;
    if (x + popupWidth > availGeo.right() - sideMargin)
        x = availGeo.right() - sideMargin - popupWidth;

    // 🔍 herausfinden, wo ein Panel reserviert
    bool panelTop    = availGeo.top()    > screenGeo.top();
    bool panelBottom = availGeo.bottom() < screenGeo.bottom();
    bool panelLeft   = availGeo.left()   > screenGeo.left();
    bool panelRight  = availGeo.right()  < screenGeo.right();

    int y = 0;

    if (panelBottom) {
        // KDE/Cinnamon/Mint: Panel unten → Fenster direkt darüber
        y = availGeo.bottom() - popupHeight;
    } else if (panelTop) {
        // Ubuntu/GNOME: Panel oben → Fenster direkt darunter
        y = availGeo.top();
    } else {
        // kein klares Panel oben/unten → nahe am Mausklick platzieren
        y = clickPos.y() - popupHeight / 2;
        if (y < availGeo.top())
            y = availGeo.top();
        if (y + popupHeight > availGeo.bottom())
            y = availGeo.bottom() - popupHeight;
    }

    // (links/rechts-Panel ignorieren wir hier bewusst – X ist schon an der Maus orientiert)

    window->resize(popupWidth, popupHeight);
    window->setX(x);
    window->setY(y);

    window->show();
    window->raise();
    window->requestActivate();
}

// Gemeinsame Funktion: Hauptfenster anzeigen oder nach vorne holen
static void showOrActivateMainWindow(QWindow *window)
{
    if (!window)
        return;

    Qt::WindowStates state = window->windowState();

    // Wenn minimiert → wiederherstellen
    if (state.testFlag(Qt::WindowMinimized)) {
        window->setWindowState(Qt::WindowNoState);
    }

    // Wenn nicht sichtbar → als Popup an der Leiste anzeigen
    if (!window->isVisible()) {
        showPopupOverBottomPanel(window);
        return;
    }

    // Wenn sichtbar → nur nach vorne holen, NICHT verstecken
    window->raise();
    window->requestActivate();
    window->setVisible(true);   // zur Sicherheit
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Prüfen, ob wir mit --autostart gestartet wurden
    bool startedFromAutostart = false;
    QStringList args = app.arguments();
    if (args.contains("--autostart")) {
        startedFromAutostart = true;
    }

    // Single-Instance
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

    // Icon-Theme-Verhalten
    QString currentTheme = QIcon::themeName();
    if (currentTheme.isEmpty()) {
        QIcon::setThemeName("hicolor");
        qDebug() << "Kein System-Icon-Theme gesetzt. Fallback auf 'hicolor'.";
    } else {
        qDebug() << "System-Icon-Theme wird verwendet:" << currentTheme;
    }

    // Prüfen, ob System-Tray verfügbar ist
    bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    /*trayAvailable = false;   // TEST: so tun, als gäbe es keinen Tray */

    // Prüfen, ob wir unter GNOME/Ubuntu laufen
    QString desktopEnv = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
    qDebug() << "XDG_CURRENT_DESKTOP =" << desktopEnv;

    bool isGnomeLike =
            desktopEnv.contains("GNOME", Qt::CaseInsensitive) ||
            desktopEnv.contains("ubuntu", Qt::CaseInsensitive);

    if (!trayAvailable) {
        qWarning() << "System Tray nicht verfügbar!";
    }

    // Verhalten beim letzten Fenster
    if (trayAvailable) {
        app.setQuitOnLastWindowClosed(false);
    } else {
        app.setQuitOnLastWindowClosed(true);
    }

    QQmlApplicationEngine engine;

    // C++-Objekte
    AutostartManager autostartManager;
    FavoriteBackend backend;

    // Kontext-Properties
    engine.rootContext()->setContextProperty("autostartManager", &autostartManager);
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.rootContext()->setContextProperty("trayAvailable", trayAvailable);
    engine.rootContext()->setContextProperty("startedFromAutostart", startedFromAutostart);
    engine.rootContext()->setContextProperty("isGnomeLike", isGnomeLike);

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

        // Startverhalten
        // - kein Tray -> Fenster zeigen
        // - Autostart + "nur Tray" -> NICHT zeigen
        // - sonst -> Fenster zeigen
        if (!trayAvailable
            || !startedFromAutostart
            || !autostartManager.startOnlyTray()) {
            window->show();
        }
    }

    // Tray-Icon & Menü
QSystemTrayIcon *trayIcon = nullptr;
QMenu *trayMenu = nullptr;

if (trayAvailable) {

    trayIcon = new QSystemTrayIcon(&app);
    trayIcon->setIcon(appIcon);
    trayIcon->setToolTip("FavList");

    trayMenu = new QMenu();

    QAction *showAction     = new QAction("Öffnen", trayMenu);
    QAction *settingsAction = new QAction("Einstellungen...", trayMenu);
    QAction *quitAction     = new QAction("Beenden", trayMenu);

    trayMenu->addAction(showAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    // Kontextmenü dem Tray-Icon zuweisen (für Rechtsklick)
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Nur Tray-Icon anzeigen, wenn Autostart + "nur Tray"
    if (startedFromAutostart
        && autostartManager.startOnlyTray()
        && window) {
        window->hide();
    }

    // Anfangstext abhängig davon, ob Fenster sichtbar
    if (window) {
        showAction->setText(window->isVisible() ? "Schließen" : "Öffnen");

        // Sichtbarkeitsänderung -> Text automatisch aktualisieren
        QObject::connect(window, &QWindow::visibleChanged,
                         &app, [showAction](bool visible) {
            showAction->setText(visible ? "Schließen" : "Öffnen");
        });
    }

    // Menüpunkt "Öffnen/Schließen" -> toggelt Fenster per QML
    QObject::connect(showAction, &QAction::triggered, &app, [window]() {
        if (!window)
            return;

        QMetaObject::invokeMethod(window, "toggleVisibility",
                                  Qt::QueuedConnection);
    });

    // Menüpunkt "Einstellungen..."
    QObject::connect(settingsAction, &QAction::triggered, &app, [window]() {
        if (!window)
            return;

        QMetaObject::invokeMethod(window, "openSettings",
                                  Qt::QueuedConnection);
    });

    // Klick aufs Tray-Icon (Links/Doppelklick)
    QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                     &app, [window](QSystemTrayIcon::ActivationReason reason) {
        switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick: {
            if (!window)
                return;

            if (window->isVisible()) {
                window->hide();
            } else {
                // Popup an Panel-Position
                showPopupOverBottomPanel(window);
            }
            break;
        }
        default:
            break;
        }
    });

    // Menüpunkt "Beenden"
    QObject::connect(quitAction, &QAction::triggered,
                     &app, &QCoreApplication::quit);
}


    return app.exec();
}
