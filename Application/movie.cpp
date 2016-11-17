#include "movie.h"
#include <QMap>
#include "tmdbapi.h"
#include <QDebug>

static QMap<int,Movie*> movies;

Movie *Movie::getMovie(int id) {
    //qDebug() << QString("getMovie(%1)").arg(id);
    if (!movies.contains(id)) {
        movies[id] = new Movie(id);
    }
    return movies[id];
}

Movie::Movie(int movieId) {
    //qDebug() << QString("new Movie(%1)").arg(movieId);
    id = movieId;
    TMDBQuery *tmdbQuery = TMDBQuery::newMovieQuery(id);
    connect(tmdbQuery, SIGNAL(response(QJsonDocument)), this, SLOT(handleMovieResults(QJsonDocument)));
    tmdbQuery->send();
}

void Movie::handleMovieResults(QJsonDocument doc) {
    //qDebug() << QString("handleMovieResults(doc)");
    data = doc.object();
    emit hasChanged();
}
