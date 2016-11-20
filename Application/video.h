#ifndef MOVIE_H
#define MOVIE_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

enum VideoType {
    movie,
    tvEpisode
};

class Video: public QObject
{
    Q_OBJECT
public:
    VideoType type;
    int id;
    static Video* getMovie(int id);
    static Video* getTVEpisode(int TVId, int season, int episode);
    QJsonObject data;

signals:
    void hasChanged();

private:
    Video(int movieId);
    Video(int tvId, int season, int episode);

private slots:
    void handleResults(QJsonDocument doc);
};

#endif // MOVIE_H
