#include "tvepisode.h"
#include <QMap>
#include "tmdbapi.h"
#include "tvseason.h"

struct EpisodeKey{
    int id,s,e;
};

bool operator<(const EpisodeKey &a, const EpisodeKey &b) {
    if (a.id != b.id) return a.id < b.id;
    if (a.s != b.s) return a.s < b.s;
    return a.e < b.e;
}

static QMap<EpisodeKey,TVEpisode*> tvEpisodes;

TVEpisode *TVEpisode::getTVEpisode(int TVId, int season, int episode) {
    EpisodeKey key = EpisodeKey{TVId,season,episode};
    if (!tvEpisodes.contains(key)) {
        tvEpisodes[key] = new TVEpisode(TVId, season, episode);
    }
    return tvEpisodes[key];
}

QString TVEpisode::getTitleText() {
    return QString("S%1E%2: %3")
            .arg(data["season_number"].toInt(),2,10,QLatin1Char('0'))
            .arg(data["episode_number"].toInt(),2,10,QLatin1Char('0'))
            .arg(data["name"].toString());
}

QString TVEpisode::getDate() {
    return data["air_date"].toString();
}

TVEpisode::TVEpisode(int tvId, int season, int episode)
    :ModelNode(TVSeason::getTVSeason(tvId,season)){
    setTMDBQuery(TMDBQuery::newTvEpisodeQuery(tvId, season, episode));
}
