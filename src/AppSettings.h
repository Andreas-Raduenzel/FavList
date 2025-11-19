#pragma once

#include <QObject>

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(bool autostartEnabled READ autostartEnabled WRITE setAutostartEnabled NOTIFY autostartEnabledChanged)
    Q_PROPERTY(bool startMinimized READ startMinimized WRITE setStartMinimized NOTIFY startMinimizedChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    QString themeMode() const;
    void setThemeMode(const QString &mode);

    bool autostartEnabled() const;
    void setAutostartEnabled(bool enabled);

    bool startMinimized() const;
    void setStartMinimized(bool enabled);

signals:
    void themeModeChanged();
    void autostartEnabledChanged();
    void startMinimizedChanged();

private:
    void load();
    void save();
    void updateAutostart();   // .desktop schreiben/löschen

    QString m_themeMode;      // "system", "light", "dark"
    bool m_autostartEnabled;
    bool m_startMinimized;
};
