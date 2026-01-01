#include "autostartmanager.h"

#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>

AutostartManager::AutostartManager(QObject *parent)
    : QObject(parent)
{
}

QString AutostartManager::autostartFilePath() const
{
    // ~/.config
    const QString configDir =
            QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    // ~/.config/autostart
    const QString autostartDir = configDir + "/autostart";

    QDir dir;
    if (!dir.exists(autostartDir)) {
        dir.mkpath(autostartDir);
    }

    // Einheitlicher Autostart-Dateiname
    return autostartDir + "/favlist.desktop";
}

bool AutostartManager::isAutostartEnabled() const
{
    return QFile::exists(autostartFilePath());
}

void AutostartManager::setAutostartEnabled(bool enabled)
{
    const QString filePath = autostartFilePath();

    // 🧹 Altlast aus früheren Versionen entfernen
    QFile::remove(
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + "/autostart/favorite-v30.desktop"
    );

    if (enabled) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }

        const QString execPath = QCoreApplication::applicationFilePath();
        const QString quotedExec = "\"" + execPath + "\"";

        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        out << "Type=Application\n";
        out << "Name=FavList\n";
        out << "Exec=" << quotedExec << " --autostart\n";
        out << "Icon=favlist\n";
        out << "X-GNOME-Autostart-enabled=true\n";
        out << "X-KDE-autostart-after=panel\n";

        file.close();
    } else {
        QFile::remove(filePath);
    }
}

bool AutostartManager::startOnlyTray() const
{
    QSettings settings;
    return settings.value("startOnlyTray", false).toBool();
}

void AutostartManager::setStartOnlyTray(bool enabled)
{
    QSettings settings;
    settings.setValue("startOnlyTray", enabled);
}
