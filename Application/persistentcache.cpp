#include "persistentcache.h"
#include <QDebug>

PersistentCache::PersistentCache(QObject *parent) : QObject(parent)
{
    storeCacheTimer.setSingleShot(true);
    storeCacheTimer.setInterval(3000);
    connect(&storeCacheTimer, SIGNAL(timeout()), this, SLOT(storeCache()));
    loadCache();
}

bool PersistentCache::contains(QByteArray key)
{
    return cache.contains(key);
}

QByteArray PersistentCache::value(QByteArray key)
{
    return cache.value(key);
}

void PersistentCache::insert(QByteArray key, QByteArray value)
{
    cache.insert(key, value);
}

void PersistentCache::loadCache()
{
    qDebug() << "loadCache()";
}

void PersistentCache::storeCache()
{
    qDebug() << "storeCache()";
}
