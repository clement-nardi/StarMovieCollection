#include "tmdbapi.h"
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QUrlQuery>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
#include <QMultiMap>

const QString apiURL = "api.themoviedb.org";
const QString api_key = "f6af9d437bd8f1d90b9720f742dbce40";

QNetworkAccessManager * TMDBQuery::manager = new QNetworkAccessManager();
PersistentCache TMDBQuery::cache;


QLinkedList<QString> TMDBQuery::queue;
QMultiMap<QString, TMDBQuery*> TMDBQuery::queries;
QMap<QString, bool> TMDBQuery::priorities;
int TMDBQuery::maxQueries = 40;
int TMDBQuery::sentQueries = 0;


TMDBQuery::TMDBQuery(QString path, QMap<QString, QString> parameters){
    url.setScheme("https");
    url.setAuthority(apiURL);
    query.addQueryItem("api_key", api_key);
    query.addQueryItem("language", "fr-fr");

    url.setPath(path);

    QMapIterator<QString, QString> i(parameters);
    while (i.hasNext()) {
        i.next();
        query.addQueryItem(i.key(), i.value());
    }
    url.setQuery(query);
    ////qDebug() << url.toDisplayString();
}

TMDBQuery *TMDBQuery::newSearchQuery(QString keywords, int page) {
    ////qDebug() << QString("newSearchQuery(%1, %2)").arg(keywords).arg(page);
    //https://api.themoviedb.org/3/search/multi?api_key=f6af9d437bd8f1d90b9720f742dbce40&language=fr-FR&query=les%20%C3%A9vad%C3%A9s&page=1&include_adult=true
    QMap<QString,QString> p;
    p["query"]          = keywords;
    p["include_adult"]  = "true";
    p["page"]           = QString("%1").arg(page);

    return new TMDBQuery("/3/search/multi", p);
}

TMDBQuery *TMDBQuery::newMovieQuery(int movieID) {
    ////qDebug() << QString("newMovieQuery(%1)").arg(movieID);
    //https://api.themoviedb.org/3/movie/278?api_key=f6af9d437bd8f1d90b9720f742dbce40&language=en-US

    return new TMDBQuery(QString("/3/movie/%1").arg(movieID));
}

TMDBQuery *TMDBQuery::newTvEpisodeQuery(int id, int season, int episode) {
    //https://api.themoviedb.org/3/tv/1450/season/1/episode/1?api_key=f6af9d437bd8f1d90b9720f742dbce40&language=en-US
    return new TMDBQuery(QString("/3/tv/%1/season/%2/episode/%3")
                         .arg(id)
                         .arg(season)
                         .arg(episode));
}

void TMDBQuery::send(bool hasPriority) {
    QByteArray key = url.toEncoded();
    if (cache.contains(key)) {
        emit response(QJsonDocument::fromJson(cache.value(key)));
    } else {
        queries.insert(key,this);
        if (hasPriority) {
            if (!priorities.contains(key)) {
                queue.prepend(key);
                priorities[key] = true;
            } else {
                if (priorities[key] == false) {
                    // move up in the queue
                    queue.removeOne(key);
                    queue.prepend(key);
                    priorities[key] = true;
                } else {
                    // already priority request for this url
                }
            }
        } else {
            if (!priorities.contains(key)) {
                queue.append(key);
                priorities[key] = false;
            }
        }
        processQueue();
    }
}

QTimer *TMDBQuery::waitTimer() {
    static QTimer *timer = NULL;
    if (timer == NULL) {
        timer = new QTimer();
        timer->setSingleShot(true);
        timer->connect(timer,&QTimer::timeout,TMDBQuery::processQueue);
    }
    return timer;
}

QTimer *TMDBQuery::resetTimer() {
    static QTimer *timer = NULL;
    if (timer == NULL) {
        timer = new QTimer();
        timer->setSingleShot(true);
        timer->connect(timer,&QTimer::timeout,TMDBQuery::resetQueryCount);
    }
    return timer;
}

void TMDBQuery::resetQueryCount() {
    //qDebug() << "-->resetQueryCount()";
    sentQueries = 0;
    processQueue();
}

void TMDBQuery::processQueue() {
    //qDebug() << "-->processQueue()";
    if (!waitTimer()->isActive()) {
        //qDebug() << "   queue.size()=" << queue.size() << " sent/max=" << sentQueries << "/" << maxQueries;
        while (sentQueries < maxQueries && !queue.isEmpty()) {
            QString key = queue.first();
            queue.removeFirst();
            sentQueries++;
            //qDebug() << "   pop() " << sentQueries << " queries sent.";
            if (!resetTimer()->isActive()) {
                resetTimer()->start(10000);
            }
            TMDBQuery *query = queries.value(key,NULL);
            if (query != NULL) {
                query->reply = manager->get(QNetworkRequest(key));
                // connect to one of the queries
                connect(query->reply, SIGNAL(finished()),
                        query , SLOT  (treatResponse()) );
            } else {
                qWarning() << "Couldn't find a query associated to this url: " << key;
            }
        }
    }
}

float TMDBQuery::getCacheHitRatio() {
    return cache.hitRatio();
}

void TMDBQuery::treatResponse() {
    ////qDebug() << "--> treatResponse()";
    QByteArray key = url.toEncoded();
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    ////qDebug() << data;
    bool tryAgain = false;
    if (reply->error() == QNetworkReply::NoError) {
        cache.insert(key,data);
    } else {
        QRegularExpressionMatch limit = QRegularExpression("is over the allowed limit of ([0-9]*)\\.").match(QString(data));
        if (limit.hasMatch()) {
            maxQueries = limit.captured(1).toInt();
            //qDebug() << "Maximum queries = " << maxQueries;
        }
        if (reply->hasRawHeader("Retry-After")) {
            bool ok;
            int nbSec = reply->rawHeader("Retry-After").toInt(&ok);
            if (ok) {
                //qDebug() << "Wait for " << nbSec << " before sending new requests";
                waitTimer()->start(nbSec*1000);
            }
            tryAgain = true;
        } else {
            qWarning() << "problem with query: " << key;
            qWarning() << "raw answer: " << data;
            if (doc.isObject()) {
                QJsonValue status_raw = doc.object()["status_code"];
                if (!status_raw.isUndefined() && !status_raw.isNull()) {
                    int status_code = status_raw.toInt(-1);
                    if (status_code == 34) {
                        //The resource you requested could not be found.
                        //This is a valid server answer (for example: requested a tv id instead of movie)
                        cache.insert(key,data);
                    } else {
                        qWarning() << "un-catched status_code: " << status_code;
                    }
                } else {
                    QJsonValue errors_raw = doc.object()["errors"];
                    if (!errors_raw.isUndefined() && !errors_raw.isNull() && errors_raw.isArray()) {
                        QJsonArray errors = errors_raw.toArray();
                        bool onlyCatched = true;
                        for (int i = 0; i < errors.size(); i++) {
                            QString error = errors[i].toString();
                            if (error == "query must be provided") {
                            } else {
                                onlyCatched = false;
                                qWarning() << "un-catched error: " << error;
                            }
                        }
                        if (onlyCatched) {
                            cache.insert(key,data);
                        }
                    } else {
                        qWarning() << "no status_code or errors defined";
                    }
                }
            } else {
                qWarning() << "answer is not a JSON object";
            }
            emit response(doc);
            reply->deleteLater();
        }
        ////qDebug() << (int)(reply->error()) << reply->error();
        ////qDebug() << reply->rawHeaderPairs();
        ////qDebug() << reply->errorString();
    }
    if (!tryAgain) {
        QList<TMDBQuery*> q = queries.values(key);
        //qDebug() << "Emitting " << q.size() << " responses";
        for (int i = 0; i < q.size(); i++) {
            q.at(i)->response(doc);
        }
        queries.remove(key);
        emit response(doc);
    } else {
        if (priorities[key]) {
            queue.prepend(key);
        } else {
            queue.append(key);
        }
    }
    reply->deleteLater();

    ////qDebug() << "<-- treatResponse()";
}
