#include "FavoriteBackend.h"

#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QMimeDatabase>
#include <QMimeType>
#include <QIcon>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

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
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                   + "/favorites.json";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray())
        return;

    QJsonArray array = doc.array();

    m_favorites.clear();
    for (const QJsonValue &value : array) {
        if (value.isString()) {
            QString fav = value.toString();
            if (!fav.isEmpty())
                m_favorites.append(fav);
        }
    }

    // Falls beim Programmstart schon ein QML-Binding existiert
    emit favoritesChanged();
}

void FavoriteBackend::save()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);

    QFile file(path + "/favorites.json");
    if (!file.open(QIODevice::WriteOnly))
        return;

    QJsonArray array;
    for (const QString &fav : m_favorites) {
        array.append(fav);  // jeder Favorit ist ein Eintrag im JSON-Array
    }

    QJsonDocument doc(array);
    file.write(doc.toJson());
}

QString FavoriteBackend::iconPathForFile(const QString &path)
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(path);
    QIcon icon = QIcon::fromTheme(type.iconName());

    if (!icon.isNull()) {
        QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                          + "/icon_" + QFileInfo(path).suffix() + ".png";
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

    QString item = m_favorites.at(from);
    m_favorites.removeAt(from);

    // Wenn der ursprüngliche Index kleiner war, verschiebt sich das Ziel um eins nach vorne
    if (from < to)
        --to;

    m_favorites.insert(to, item);

    emit favoritesChanged();  // QML über neue Reihenfolge informieren
    save();                   // neue Reihenfolge persistent speichern
}

void FavoriteBackend::addFavoriteFromUrl(const QUrl &url)
{
    const QString path = url.toLocalFile();
    if (path.isEmpty())
        return;

    addFavorite(path);  // nutzt deine vorhandene Logik + save() etc.
}

