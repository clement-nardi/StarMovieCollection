#include "tvshow.h"
#include "tmdbapi.h"

static QMap<int,TVShow*> tvShows;


TVShow *TVShow::getTVShow(int tvId) {
    if (!tvShows.contains(tvId)) {
        tvShows[tvId] = new TVShow(tvId);
    }
    return tvShows[tvId];
}

QString TVShow::getTitleText() {
    QString title = data["name"].toString();
    QString oriTitle = data["original_name"].toString();
    QString out = oriTitle;
    if (title != oriTitle) {
        out += " (" + title + ")";
    }
    return out;
}

QString TVShow::getDate() {
    return QString("%1 to %2")
            .arg(data["first_air_date"].toString())
            .arg(data["last_air_date"].toString());
}

TVShow::TVShow(int tvId):ModelNode() {
    setTMDBQuery(TMDBQuery::newTvShowQuery(tvId));
}
