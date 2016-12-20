#ifndef MOVIE_H
#define MOVIE_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>
#include "modelnode.h"

class Movie: public ModelNode
{
    Q_OBJECT
public:
    static Movie* getMovie(int id);

    QString getTitleText();
    QString getDate();

    QString getHtmlTitle();
    QString getHtmlDate();


private:
    Movie(int movieId);

};

#endif // MOVIE_H
