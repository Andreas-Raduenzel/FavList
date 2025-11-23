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

    // Name der Autostart-Datei
    return autostartDir + "/favorite-v30.desktop";
}

bool AutostartManager::isAutostartEnabled() const
{
    return QFile::exists(autostartFilePath());
}

void AutostartManager::setAutostartEnabled(bool enabled)
{
    const QString filePath = autostartFilePath();

    if (enabled) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }

        const QString execPath = QCoreApplication::applicationFilePath();
        // Zur Sicherheit in Anführungszeichen, falls Leerzeichen im Pfad sind
        const QString quotedExec = "\"" + execPath + "\"";

        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        out << "Type=Application\n";
        out << "Name=Favorite v30\n";
        out << "Exec=" << quotedExec << " --autostart\n";
        out << "Icon=favorite\n";
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


