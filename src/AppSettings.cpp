#include "AppSettings.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

AppSettings::AppSettings(QObject *parent)
    : QObject(parent),
      m_themeMode("system"),
      m_autostartEnabled(false),
      m_startMinimized(false)
{
    load();
    updateAutostart();
}

void AppSettings::load()
{
    // Settings orientieren sich an den global gesetzten App-Daten (main.cpp)
    QSettings s(QCoreApplication::organizationName(),
                QCoreApplication::applicationName());

    m_themeMode        = s.value("themeMode", "system").toString();
    m_autostartEnabled = s.value("autostartEnabled", false).toBool();
    m_startMinimized   = s.value("startMinimized", false).toBool();
}

void AppSettings::save()
{
    QSettings s(QCoreApplication::organizationName(),
                QCoreApplication::applicationName());

    s.setValue("themeMode",        m_themeMode);
    s.setValue("autostartEnabled", m_autostartEnabled);
    s.setValue("startMinimized",   m_startMinimized);
}

QString AppSettings::themeMode() const
{
    return m_themeMode;
}

void AppSettings::setThemeMode(const QString &mode)
{
    if (mode == m_themeMode)
        return;

    m_themeMode = mode;
    save();
    emit themeModeChanged();
}

bool AppSettings::autostartEnabled() const
{
    return m_autostartEnabled;
}

void AppSettings::setAutostartEnabled(bool enabled)
{
    if (enabled == m_autostartEnabled)
        return;

    m_autostartEnabled = enabled;
    save();
    updateAutostart();
    emit autostartEnabledChanged();
}

bool AppSettings::startMinimized() const
{
    return m_startMinimized;
}

void AppSettings::setStartMinimized(bool enabled)
{
    if (enabled == m_startMinimized)
        return;

    m_startMinimized = enabled;
    save();
    updateAutostart(); // Exec ggf. mit --hidden setzen
    emit startMinimizedChanged();
}

void AppSettings::updateAutostart()
{
    // ~/.config/autostart
    const QString configDir    = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const QString autostartDir = configDir + "/autostart";
    QDir().mkpath(autostartDir);

    const QString desktopFile = autostartDir + "/favlist.desktop";

    if (!m_autostartEnabled) {
        QFile::remove(desktopFile);
        return;
    }

    QFile f(desktopFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString exec = QCoreApplication::applicationFilePath();
    if (m_startMinimized)
        exec += " --hidden";

    QTextStream out(&f);
    out << "[Desktop Entry]\n";
    out << "Type=Application\n";
    out << "Exec=" << exec << "\n";
    out << "Hidden=false\n";
    out << "X-GNOME-Autostart-enabled=true\n";
    out << "Name=FavList\n";
    out << "Comment=Deine persönliche Favoritenliste\n";
    out << "Icon=favlist\n";   // nutzt /usr/share/icons/hicolor/.../favlist.svg
}
