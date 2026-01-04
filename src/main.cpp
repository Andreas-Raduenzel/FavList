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

#include <QTimer>


#include "FavoriteBackend.h"
#include "autostartmanager.h"

#include <QFile>

#include <QPointer>



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

    app.setQuitOnLastWindowClosed(false);


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

    QIcon appIcon(":/resources/icons/appicon.svg");
    app.setWindowIcon(appIcon);

    // Icon-Theme-Verhalten
    QString currentTheme = QIcon::themeName();
    if (currentTheme.isEmpty()) {
        QIcon::setThemeName("hicolor");
        qDebug() << "Kein System-Icon-Theme gesetzt. Fallback auf 'hicolor'.";
    } else {
        qDebug() << "System-Icon-Theme wird verwendet:" << currentTheme;
    }

    // Prüfen, ob System-Tray verfügbar ist (Qt-Meinung)
    bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    /*trayAvailable = false;   // TEST: so tun, als gäbe es keinen Tray */
 

    if (!trayAvailable) {
        qWarning() << "System Tray nicht verfügbar!";
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

    QString qmlPath = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/../qml/main.qml"
    );

    if (!QFile::exists(qmlPath)) {
        qmlPath = "/usr/share/favlist/qml/main.qml";
    }

    engine.load(QUrl::fromLocalFile(qmlPath));

    if (engine.rootObjects().isEmpty())
        return -1;

    QObject *topLevel = engine.rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(topLevel);
    QPointer<QWindow> winPtr(window);

    if (winPtr) {
    winPtr->setIcon(appIcon);

    if (!trayAvailable) {
    winPtr->show();
    } else {
        if (startedFromAutostart && autostartManager.startOnlyTray()) {
            winPtr->hide(); // Fenster erstmal verstecken – Tray kommt ggf. gleich/gleich später
        } else {
            showPopupOverBottomPanel(winPtr);
        }
    }

    } // <- WICHTIG: schließt if(winPtr)

// Tray-Icon & Menü (Lazy / Retry für Autostart)
QSystemTrayIcon *trayIcon = nullptr;
QMenu *trayMenu = nullptr;

// Aktionen außerhalb anlegen, damit wir sie nicht mehrfach erzeugen
QAction *showAction = nullptr;
QAction *settingsAction = nullptr;
QAction *quitAction = nullptr;

auto initTrayOnce = [&]() {
    if (trayIcon) {
        return; // schon erstellt
    }

    // Tray/Indicator muss JETZT wirklich verfügbar sein
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "Tray noch nicht verfügbar...";
        return;
    }

    // Tray ist da -> jetzt erstellen wir alles
    trayIcon = new QSystemTrayIcon(&app);
    trayIcon->setIcon(appIcon);
    trayIcon->setToolTip("FavList");

    trayMenu = new QMenu();
    showAction     = new QAction("Öffnen", trayMenu);
    settingsAction = new QAction("Einstellungen...", trayMenu);
    quitAction     = new QAction("Beenden", trayMenu);

    trayMenu->addAction(showAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Text abhängig von Fenster-Sichtbarkeit
    if (winPtr) {
        showAction->setText(winPtr->isVisible() ? "Schließen" : "Öffnen");

        QObject::connect(winPtr, &QWindow::visibleChanged,
                 &app, [showAction](bool visible) {
    showAction->setText(visible ? "Schließen" : "Öffnen");
    });

    }

    // Menüpunkt "Öffnen/Schließen" -> toggelt Fenster per QML
    QObject::connect(showAction, &QAction::triggered, &app, [winPtr]() {
        if (!winPtr)
            return;

        QMetaObject::invokeMethod(winPtr, "toggleVisibility",
                                  Qt::QueuedConnection);
    });

    // Menüpunkt "Einstellungen..."
    QObject::connect(settingsAction, &QAction::triggered, &app, [winPtr]() {
        if (!winPtr)
            return;

        QMetaObject::invokeMethod(winPtr, "openSettings",
                                  Qt::QueuedConnection);
    });

    // Klick aufs Tray-Icon (Links/Doppelklick)
    QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                     &app, [winPtr](QSystemTrayIcon::ActivationReason reason) {
        switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick: {
            if (!winPtr)
                return;

            if (winPtr->isVisible()) {
                winPtr->hide();
            } else {
                showPopupOverBottomPanel(winPtr);
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

    qDebug() << "Tray erfolgreich erstellt.";

    // ✅ Wenn Autostart + "nur Tray", dann Fenster sicher verstecken,
    // aber erst JETZT (wo Tray wirklich existiert).
    if (startedFromAutostart && autostartManager.startOnlyTray() && winPtr) {
        winPtr->hide();
    }
};

// Sofort versuchen
initTrayOnce();

// Retry: Autostart-GNOME/Ubuntu braucht oft ein bisschen, bis der Indicator da ist
QTimer::singleShot(1000, &app, [=]() { initTrayOnce(); });
QTimer::singleShot(2000, &app, [=]() { initTrayOnce(); });
QTimer::singleShot(4000, &app, [=]() { initTrayOnce(); });
QTimer::singleShot(8000, &app, [=]() { initTrayOnce(); });


// Optional: Falls nach 8 Sekunden immer noch kein Tray da ist und die App im Autostart
// "nur Tray" sollte, zeigen wir das Fenster, damit sie nicht "unsichtbar" bleibt.
QTimer::singleShot(9000, &app, [&]() {
    if (startedFromAutostart && autostartManager.startOnlyTray()) {
        if (!trayIcon && winPtr) {
            qWarning() << "Kein Tray nach 9s – zeige Fenster, damit App sichtbar ist.";
            winPtr->show();
        }
    }
});




    return app.exec();
}
