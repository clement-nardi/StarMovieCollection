#include "tmdbapi.h"
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QUrlQuery>
#include <QList>
#include <QPair>

const QString apiURL = "api.themoviedb.org";
const QString api_key = "f6af9d437bd8f1d90b9720f742dbce40";

QNetworkAccessManager * TMDBQuery::manager = new QNetworkAccessManager();
PersistentCache TMDBQuery::cache;

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
    //qDebug() << url.toDisplayString();
}

TMDBQuery *TMDBQuery::newSearchQuery(QString keywords, int page) {
    //https://api.themoviedb.org/3/search/multi?api_key=f6af9d437bd8f1d90b9720f742dbce40&language=fr-FR&query=les%20%C3%A9vad%C3%A9s&page=1&include_adult=true
    QMap<QString,QString> p;
    p["query"]          = keywords;
    p["include_adult"]  = "true";
    p["page"]           = QString("%1").arg(page);

    return new TMDBQuery("/3/search/multi", p);
}

TMDBQuery *TMDBQuery::newMovieQuery(int movieID) {
    //https://api.themoviedb.org/3/movie/278?api_key=f6af9d437bd8f1d90b9720f742dbce40&language=en-US

    return new TMDBQuery(QString("/3/movie/%1").arg(movieID));
}

void TMDBQuery::send() {
    QByteArray key = url.toEncoded();
    if (cache.contains(key)) {
        emit response(QJsonDocument::fromJson(cache.value(key)));
    } else {
        reply = manager->get(QNetworkRequest(url));
        connect(reply, SIGNAL(finished()),
                this , SLOT  (treatResponse()) );
    }
}

float TMDBQuery::getCacheHitRatio() {
    return cache.hitRatio();
}

void TMDBQuery::treatResponse() {
    QByteArray data = reply->readAll();
    if (reply->error() == QNetworkReply::NoError) {
        cache.insert(url.toEncoded(),data);
        //qDebug() << response;
    } else {
        //TODO: error handling?
        qDebug() << reply->errorString();
    }
    emit response(QJsonDocument::fromJson(data));

    reply->deleteLater();
}
