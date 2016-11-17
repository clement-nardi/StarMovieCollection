#ifndef MOVIE_H
#define MOVIE_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>

class Movie: public QObject
{
    Q_OBJECT
public:
    int id;
    static Movie* getMovie(int id);
    QJsonObject data;

signals:
    void hasChanged();

private:
    Movie(int movieId);

private slots:
    void handleMovieResults(QJsonDocument doc);
};

#endif // MOVIE_H
