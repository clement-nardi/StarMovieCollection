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
#include "persistentcache.h"

class TMDBQuery: public QObject {
    Q_OBJECT

public:
    static TMDBQuery * NewSearchQuery(QString keywords, int page = 1);
    static TMDBQuery * NewMovieQuery(int movieID);
    void Send();

signals:
    void Response(QJsonDocument);




private:
    TMDBQuery(QString path, QMap<QString,QString> parameters = QMap<QString,QString>());
    static QNetworkAccessManager *manager;
    QUrl url;
    QUrlQuery query;
    QNetworkReply * reply;
    int page;

    static PersistentCache *cache;

private slots:

    void TreatResponse();

};

#endif // TMDBAPI_H
