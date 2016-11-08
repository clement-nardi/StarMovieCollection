#ifndef PERSISTENTCACHE_H
#define PERSISTENTCACHE_H

#include <QMap>
#include <QByteArray>
#include <QTimer>

class PersistentCache: QObject {
    Q_OBJECT
public:
    explicit PersistentCache(QObject *parent);

    bool contains(QByteArray key);
    QByteArray value(QByteArray key);
    void insert(QByteArray key, QByteArray value);

private:
    void loadCache();
    QTimer storeCacheTimer;
    QMap<QByteArray, QByteArray> cache;

private slots:
    void storeCache();
};

#endif // PERSISTENTCACHE_H
