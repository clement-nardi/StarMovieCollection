#include "movie.h"
#include <QMap>
#include "tmdbapi.h"
#include <QDebug>
#include "profiler.h"
#include <QRegularExpression>
#include "patterns.h"

static QMap<int,Movie*> movies;

Movie *Movie::getMovie(int id) {
    //qDebug() << QString("getMovie(%1)").arg(id);
    if (!movies.contains(id)) {
        movies[id] = new Movie(id);
    }
    return movies[id];
}

QString Movie::getTitleText() {
    QString title = data["title"].toString();
    QString oriTitle = data["original_title"].toString();
    QString out = oriTitle;
    if (title != oriTitle) {
        out += " (" + title + ")";
    }
    return out;
}

QString Movie::getDate() {
    return data["release_date"].toString();
}

QString Movie::getHtmlTitle() {
    profiler()->funcBegin("colorify title");
    QString title = getTitleText();
    QString filePath = childAt(0)->getPath();
    QString fileTitle = childAt(0)->getTitleText();
    QString out;
    int wordStart = -1;
    bool matched = false;
    QRegularExpression regex = pattern()["wordChar"];
    regex.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
    for (int i = 0; i < title.size()+1; i++) {
        QChar c;
        if (i < title.size()) {
            c = title[i];
        }
        if (i < title.size() && regex.match(c).hasMatch()) {
            if (wordStart == -1) {
                wordStart = i;
            }
        } else {
            if (wordStart != -1) {
                QString word = title.mid(wordStart,i-wordStart);
                QRegularExpression wordRegexp = QRegularExpression("(^|[^a-z0-9])" +
                                                                   QRegularExpression::escape(word) +
                                                                   "($|[^a-z0-9])",
                                                                   QRegularExpression::CaseInsensitiveOption);
                if (wordRegexp.match(fileTitle).hasMatch()) {
                    out.append("<span style=\" color:#2e0bd9;\">" + word + "</span>");
                    matched = true;
                } else if (wordRegexp.match(filePath).hasMatch()) {
                    out.append("<span style=\" color:#3399aa;\">" + word + "</span>");
                    matched = true;
                } else {
                    out.append(word);
                }
            }
            if (i < title.size()) {
                out.append(c);
            }
            wordStart = -1;
        }
    }
    profiler()->funcEnd("colorify title");
    if (matched) {
        return out;
    } else {
        return "";
    }
}

QString Movie::getHtmlDate() {
    profiler()->funcBegin("colorify date");
    QString date = getDate();
    int releaseYear = date.left(4).toInt();
    int fileYear = childAt(0)->getDate().toInt();
    int yearDistance = abs(releaseYear - fileYear);
    QString color;
    switch (yearDistance) {
    case 0: color = "2e0bd9"; break;
    case 1: color = "3399aa"; break;
    case 2: color = "f19855"; break;
    default: color = "ff1111";
    }
    QString coloredDate = QString("<span style=\" color:#%1;\">%2</span>%3")
            .arg(color)
            .arg(date.left(4))
            .arg(date.mid(4));
    profiler()->funcEnd("colorify date");
    return coloredDate;
}

Movie::Movie(int movieId) {
    //qDebug() << QString("new Movie(%1)").arg(movieId);
    setTMDBQuery(TMDBQuery::newMovieQuery(movieId));
}

