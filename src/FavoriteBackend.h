#ifndef FAVORITEBACKEND_H
#define FAVORITEBACKEND_H

#include <QObject>
#include <QStringList>
#include <QUrl>              // <– neu

class FavoriteBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList favorites READ favorites NOTIFY favoritesChanged)

public:
    explicit FavoriteBackend(QObject *parent = nullptr);

    Q_INVOKABLE QStringList favorites() const;
    Q_INVOKABLE void addFavorite(const QString &path);
    Q_INVOKABLE void removeFavorite(const QString &path);
    Q_INVOKABLE QString iconPathForFile(const QString &path);
    Q_INVOKABLE void moveFavorite(int from, int to);
    Q_INVOKABLE void addFavoriteFromUrl(const QUrl &url);   // <– neu

signals:
    void favoritesChanged();

private:
    QStringList m_favorites;
    void load();
    void save();
};

#endif // FAVORITEBACKEND_H
