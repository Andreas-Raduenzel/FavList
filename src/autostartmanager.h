#ifndef AUTOSTARTMANAGER_H
#define AUTOSTARTMANAGER_H

#include <QObject>

class AutostartManager : public QObject
{
    Q_OBJECT
public:
    explicit AutostartManager(QObject *parent = nullptr);

    Q_INVOKABLE bool isAutostartEnabled() const;
    Q_INVOKABLE void setAutostartEnabled(bool enabled);

private:
    QString autostartFilePath() const;
};

#endif // AUTOSTARTMANAGER_H
