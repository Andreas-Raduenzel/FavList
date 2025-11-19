#include "FavoriteBackend.h"
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QMimeDatabase>
#include <QMimeType>
#include <QIcon>
#include <QPixmap>
#include <QDir>

FavoriteBackend::FavoriteBackend(QObject *parent)
    : QObject(parent)
{
    load();
}

QStringList FavoriteBackend::favorites() const
{
    return m_favorites;
}

void FavoriteBackend::addFavorite(const QString &path)
{
    if (!m_favorites.contains(path)) {
        m_favorites.append(path);
        save();
        emit favoritesChanged();
    }
}

void FavoriteBackend::removeFavorite(const QString &path)
{
    if (m_favorites.contains(path)) {
        m_favorites.removeAll(path);
        save();
        emit favoritesChanged();
    }
}

void FavoriteBackend::load()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/favorites.txt";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (!line.isEmpty())
                m_favorites.append(line);
        }
    }
}

void FavoriteBackend::save()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);
    QFile file(path + "/favorites.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString &fav : m_favorites)
            out << fav << "\n";
    }
}

QString FavoriteBackend::iconPathForFile(const QString &path)
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(path);
    QIcon icon = QIcon::fromTheme(type.iconName());

    if (!icon.isNull()) {
        QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/icon_" + QFileInfo(path).suffix() + ".png";
        icon.pixmap(32, 32).save(tmpPath);
        return tmpPath;
    }

    return "";
}

void FavoriteBackend::moveFavorite(int from, int to)
{
    if (from < 0 || to < 0 || from >= m_favorites.size() || to >= m_favorites.size())
        return;

    if (from == to)
        return;

    // Eintrag verschieben
    QString item = m_favorites.at(from);
    m_favorites.removeAt(from);
    m_favorites.insert(to, item);

    emit favoritesChanged();  // damit QML die neue Reihenfolge sieht
    save();                   // optional: neue Reihenfolge direkt speichern
}
