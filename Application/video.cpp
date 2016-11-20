#include "video.h"
#include <QMap>
#include "tmdbapi.h"
#include <QDebug>

struct EpisodeKey{
    int id,s,e;
};

bool operator<(const EpisodeKey &a, const EpisodeKey &b) {
    if (a.id != b.id) return a.id < b.id;
    if (a.s != b.s) return a.s < b.s;
    return a.e < b.e;
}

static QMap<int,Video*> movies;
static QMap<EpisodeKey,Video*> tvEpisodes;

Video *Video::getMovie(int id) {
    //qDebug() << QString("getMovie(%1)").arg(id);
    if (!movies.contains(id)) {
        movies[id] = new Video(id);
    }
    return movies[id];
}

Video *Video::getTVEpisode(int TVId, int season, int episode) {
    EpisodeKey key = EpisodeKey{TVId,season,episode};
    if (!tvEpisodes.contains(key)) {
        tvEpisodes[key] = new Video(TVId, season, episode);
    }
    return tvEpisodes[key];
}



Video::Video(int movieId) {
    //qDebug() << QString("new Movie(%1)").arg(movieId);
    type = movie;
    id = movieId;
    TMDBQuery *tmdbQuery = TMDBQuery::newMovieQuery(id);
    connect(tmdbQuery, SIGNAL(response(QJsonDocument)), this, SLOT(handleResults(QJsonDocument)),Qt::QueuedConnection);
    tmdbQuery->send();
}

Video::Video(int tvId, int season, int episode) {
    type = tvEpisode;
    id = tvId;
    TMDBQuery *tmdbQuery = TMDBQuery::newTvEpisodeQuery(id, season, episode);
    connect(tmdbQuery, SIGNAL(response(QJsonDocument)), this, SLOT(handleResults(QJsonDocument)),Qt::QueuedConnection);
    tmdbQuery->send();
}

void Video::handleResults(QJsonDocument doc) {
    //qDebug() << QString("handleMovieResults(doc)");
    data = doc.object();
    emit hasChanged();
}
