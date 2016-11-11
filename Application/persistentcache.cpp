#include "persistentcache.h"
#include <QDebug>
#include <QDataStream>
#include <QFile>


PersistentCache::PersistentCache() : QObject() {
    storeCacheTimer.setSingleShot(true);
    storeCacheTimer.setInterval(3000);
    connect(&storeCacheTimer, SIGNAL(timeout()), this, SLOT(storeCache()));
    nbHits = 0;
    nbMiss = 0;
    loadCache();
}

PersistentCache::~PersistentCache() {
    storeCache();
    qDebug() << "Hit Ratio: " << hitRatio();
}

bool PersistentCache::contains(QByteArray key) {
    bool res = cache.contains(key);
    if (res) {
        nbHits++;
    } else {
        nbMiss++;
    }
    return res;
}

QByteArray PersistentCache::value(QByteArray key) {
    return cache.value(key);
}

void PersistentCache::insert(QByteArray key, QByteArray value) {
    cache.insert(key, value);
    storeCacheTimer.start();
}

float PersistentCache::hitRatio() {
    float ratio = 0;
    if (nbHits+nbMiss != 0) {
        ratio = (float)nbHits / (float)(nbHits+nbMiss);
    }
    return ratio;
}

void PersistentCache::loadCache() {
    qDebug() << "loadCache()";
    QFile file("cache.bin");
    file.open(QIODevice::ReadOnly);
    QDataStream ds(&file);
    ds >> cache;
}

void PersistentCache::storeCache() {
    qDebug() << "storeCache()";
    QFile file("cache.bin");
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    ds << cache;
}
