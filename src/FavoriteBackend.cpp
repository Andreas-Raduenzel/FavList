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
        array.append(fav);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson());
}

QString FavoriteBackend::iconPathForFile(const QString &path)
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(path);

    // 1️⃣ zuerst den spezifischen Icon-Namen holen, z.B. "image-jpeg"
    QString iconName = type.iconName();

    // 2️⃣ Falls es dafür kein Icon im Theme gibt → generischen Namen nehmen, z.B. "image"
    if (!QIcon::hasThemeIcon(iconName)) {
        QString generic = type.genericIconName();   // "image", "video", "text", ...
        if (!generic.isEmpty() && QIcon::hasThemeIcon(generic)) {
            iconName = generic;
        } else {
            // 3️⃣ Fallbacks, falls gar nichts geht
            if (QIcon::hasThemeIcon("text-plain"))
                iconName = "text-plain";
            else if (QIcon::hasThemeIcon("unknown"))
                iconName = "unknown";
            else
                return "";
        }
    }

    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull()) {
        // Optional etwas eindeutigere Dateinamen, damit PNG/JPG nicht alle die gleiche temp-Datei nutzen
        QString baseName = QFileInfo(path).completeBaseName();
        if (baseName.isEmpty())
            baseName = QFileInfo(path).fileName();

        QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QDir().mkpath(tmpDir); // sicherstellen, dass es den Ordner gibt

        QString tmpPath = tmpDir + "/favlist_icon_" + baseName + ".png";
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
    m_favorites.insert(to, item);

    emit favoritesChanged();
    save();
}

void FavoriteBackend::addFavoriteFromUrl(const QUrl &url)
{
    const QString path = url.toLocalFile();
    if (path.isEmpty())
        return;

    addFavorite(path);
}
