#include "tvseason.h"
#include "tmdbapi.h"
#include "tvshow.h"

struct SeasonKey{
    int id,s;
};

bool operator<(const SeasonKey &a, const SeasonKey &b) {
    if (a.id != b.id) return a.id < b.id;
    return a.s < b.s;
}

static QMap<SeasonKey,TVSeason*> tvSeasons;


TVSeason *TVSeason::getTVSeason(int TVId, int season) {
    SeasonKey key = SeasonKey{TVId,season};
    if (!tvSeasons.contains(key)) {
        tvSeasons[key] = new TVSeason(TVId, season);
    }
    return tvSeasons[key];
}

QString TVSeason::getTitleText() {
    return QString("Season %1: %3")
            .arg(data["season_number"].toInt(),2,10,QLatin1Char('0'))
            .arg(data["name"].toString());
}

QString TVSeason::getDate() {
    return data["air_date"].toString();
}

TVSeason::TVSeason(int tvId, int season)
    :ModelNode(TVShow::getTVShow(tvId)) {
    setTMDBQuery(TMDBQuery::newTvSeasonQuery(tvId, season));
}
