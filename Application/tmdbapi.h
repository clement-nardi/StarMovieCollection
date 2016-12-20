#ifndef TMDBAPI_H
#define TMDBAPI_H

#include <qstring.h>
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QMap>
#include <QTimer>
#include <QLinkedList>
#include "persistentcache.h"

class TMDBQuery: public QObject {
    Q_OBJECT

public:
    static TMDBQuery * newSearchQuery(QString keywords, int page = 1);
    static TMDBQuery * newMovieQuery(int movieID);
    static TMDBQuery * newTvShowQuery(int id);
    static TMDBQuery * newTvSeasonQuery(int id, int season);
    static TMDBQuery * newTvEpisodeQuery(int id, int season, int episode);
    void send(bool hasPriority = false);
    static float getCacheHitRatio();

signals:
    void response(QJsonDocument);




private:
    TMDBQuery(QString path, QMap<QString,QString> parameters = QMap<QString,QString>());
    static QNetworkAccessManager *manager;
    QUrl url;
    QUrlQuery query;
    QNetworkReply * reply;
    int page;

    static PersistentCache cache;

    void process();

    static QLinkedList<QString> queue;
    static QMultiMap<QString, TMDBQuery*> queries;
    static QMap<QString, bool> priorities;
    static QTimer *waitTimer();
    static QTimer *resetTimer();
    static int maxQueries;
    static int sentQueries;
    static void processQueue();
    static void resetQueryCount();
private slots:

    void treatResponse();

};

#endif // TMDBAPI_H
