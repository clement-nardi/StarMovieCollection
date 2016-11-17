#include "moviefile.h"
#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QStringList>
#include <QJsonArray>
#include <QDir>
#include "tmdbapi.h"
#include "movie.h"

MovieFile::MovieFile(QString path, int row_, QObject *parent) : QObject(parent) {
    movie = NULL;
    searchQuerySent = false;
    row = row_;
    fileInfo = QFileInfo(path);
    year = extractYear();
    titleSubString = extractTitleFromFilename();
    prettyName = computePrettyName();

    TMDBQuery *tmdbquery = TMDBQuery::newSearchQuery(getTitle());
    connect(tmdbquery, SIGNAL(response(QJsonDocument)), this, SLOT(handleSearchResults(QJsonDocument)));
    tmdbquery->send(false);
}


QString MovieFile::getTitle() {
    return titleSubString.replace("."," ");
}

class CommonPatterns: public QMap<QString,QRegularExpression> {
public:
    CommonPatterns();

    QStringList treatSubJsonObject(QJsonObject obj) {
        QStringList result;
        if (!obj["listFromFile"].isNull() || !obj["listFromFile"].isUndefined()) {
            QString filename = obj["listFromFile"].toString();
            QFile file(filename);
            if (file.exists() && file.open(QIODevice::ReadOnly)) {
                result = QString(file.readAll()).replace("\r", "").replace('\n', ' ').split(' ', QString::SkipEmptyParts);
            } else {
                qWarning() << "Missing file: " << file.fileName();
            }
        } else {
            qWarning() << "Unsuported Format, expected \"listFromFile\" instead of: " << obj;
        }
        return result;
    }
};

CommonPatterns::CommonPatterns(){
    QFile file(QCoreApplication::applicationDirPath() + "/titlePatterns.json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            QMap<QString,QStringList> patterns;
            QSet<QString> caseSensitive;
            QJsonObject obj = doc.object();
            QStringList keys = obj.keys();
            for (int i = 0; i < keys.size(); i++) {
                QString key = keys.at(i);
                if (key == "patternsThatAreCaseSensitive") {
                    QJsonArray array = obj[key].toArray();
                    for (int j = 0; j < array.size(); j++) {
                        caseSensitive.insert(array[j].toString());
                    }
                } else {
                    if (obj[key].isString()) {
                        patterns[key] << obj[key].toString();
                    } else if (obj[key].isArray()) {
                        QJsonArray array = obj[key].toArray();
                        for (int j = 0; j < array.size(); j++) {
                            if (array[j].isString()) {
                                patterns[key] << array[j].toString();
                            } else if (array[j].isObject()) {
                                patterns[key] << treatSubJsonObject(array[j].toObject());
                            }
                        }
                    } else if (obj[key].isObject()) {
                        patterns[key] << treatSubJsonObject(obj[key].toObject());
                    }
                }
            }


            QMap<QString,QString> pattern;
            for (int i = 0; i < keys.size(); i++) {
                QString key = keys[i];
                pattern[key] = QString("(%1)").arg(patterns[key].join('|'));
            }
            //find inter-references like %sep
            for (int d = 0; d < 3; d++) {
                for (int i = 0; i < keys.size(); i++) {
                    QString key1 = keys[i];
                    for (int j = 0; j < keys.size(); j++) {
                        if (i != j) {
                            QString key2 = keys[j];
                            pattern[key2].replace("%"+key1, pattern[key1]);
                        }
                    }
                }
            }
            for (int i = 0; i < keys.size(); i++) {
                if (caseSensitive.contains(keys[i])) {
                    this->insert(keys[i],QRegularExpression(pattern[keys[i]]));
                } else {
                    this->insert(keys[i],QRegularExpression(pattern[keys[i]],QRegularExpression::CaseInsensitiveOption));
                }
            }

        } else {
            qWarning() << "Expected a JSON Object in titlePatterns.json";
        }
    } else {
        qWarning() << "Missing file: " << file.fileName();
    }
    //qDebug() << *this;
}

static CommonPatterns pattern() {
    static CommonPatterns p;
    return p;
}


QString MovieFile::extractTitleFromFilename() {
    QString title;
    //qDebug() << fileInfo.absoluteFilePath();

    QString filename = fileInfo.completeBaseName();

    QDir parentFolder = fileInfo.dir();
    int seasonIdOffsetInParentFolder = -1;

    do {
        seasonIdOffsetInParentFolder = pattern()["seasonID"].match(parentFolder.dirName()).capturedStart();
    } while (seasonIdOffsetInParentFolder == -1 && parentFolder.cdUp());

    //check if it's organized like this: Series_title/season_ID/episode
    if (seasonIdOffsetInParentFolder == 0) {
        //directory named like: "Season 1 HD"
        //n-2 folder must contain the series title
        parentFolder.cdUp();
        title = extractTitle(parentFolder.dirName());
    } else if (seasonIdOffsetInParentFolder > 0) {
        // like "The Big Bang Theory Season 1"
        title = extractTitle(parentFolder.dirName());
    } else {
        QRegularExpressionMatch sMatch = pattern()["seasonID"].match(filename);
        QRegularExpressionMatch eMatch = pattern()["episodeID"].match(filename);
        QRegularExpressionMatch seMatch = pattern()["completeEpisodeID"].match(filename);
        QRegularExpressionMatch seMatchMaybe = pattern()["maybeCompleteEpisodeID"].match(filename);
        bool isEpisodeOfSerie = false;

        //qDebug() << QString("s=%1 e=%2 se=%3 se3d=%4").arg(sMatch.captured()).arg(eMatch.captured()).arg(seMatch.captured()).arg(seMatchMaybe.captured());

        //Detect Series Episode
        if ((eMatch.hasMatch() && sMatch.hasMatch()) ||
            seMatch.hasMatch() ) {
            //don't look for counters, they're expected
            //qDebug() << "Episode of a Series";
            isEpisodeOfSerie = true;
        } else if (seMatchMaybe.hasMatch()) {
            //qDebug() << "Maybe Episode of a Series?";
            // 3-digit episode IDs are source of problem
            // for instance it must be matched in:
            //    Fais.Pas.Ci,.Fais.Pas.Ca.-.201.-.Les.bonnes.manieres   // season 2 episode 1
            // But not in:
            //    N°.019.-.1961.Walt.Disney.-.Les.101.Dalmatiens.avi
            //Solution: look at nearby movie files to see if they look like series episodes

            QStringList filenames = fileInfo.dir().entryList();
            int countEpisodes = 0;
            int countMovieFiles = 0;
            for (int i = 0; i < filenames.size(); i++) {
                QString fn = filenames[i];
                if (MovieFile::isMovieFile(fn)) {
                    countMovieFiles++;
                    QRegularExpressionMatch s = pattern()["seasonID"].match(fn);
                    QRegularExpressionMatch e = pattern()["episodeID"].match(fn);
                    QRegularExpressionMatch se = pattern()["completeEpisodeID"].match(fn);
                    QRegularExpressionMatch sem = pattern()["maybeCompleteEpisodeID"].match(fn);
                    if ((e.hasMatch() && s.hasMatch()) ||
                        se.hasMatch() ||
                        sem.hasMatch()) {
                        countEpisodes++;
                    }
                }
            }
            float episodesRatio = (float)countEpisodes/(float)countMovieFiles;
            //qDebug() << QString("episodes: %1  movieFiles: %2 - ratio=%3").arg(countEpisodes).arg(countMovieFiles).arg(episodesRatio);
            if (episodesRatio > 0.9) {
                //This is likely to be an episode as well
                filename = filename.left(seMatchMaybe.capturedStart());
                isEpisodeOfSerie = true;
            } else {
                isEpisodeOfSerie = false;
            }
        }
        if (isEpisodeOfSerie) {
            //qDebug() << "Episode of a Series";
            title = extractTitle(filename);
            if (title.size() == 0) {
                //backup to folder/filename
                title = extractTitle(fileInfo.dir().dirName() + "/" + filename);
            }
        } else {
            //Movie file
            //qDebug() << "Movie File";
            int afterCounter = 0;
            int nbCharsToTrim = 0;
            while (true) {
                //detect counter
                //qDebug() << filename.mid(afterCounter);
                QRegularExpressionMatch matchedCounter = pattern()["counter"].match(filename,afterCounter==0?0:afterCounter-1);
                afterCounter = matchedCounter.capturedEnd();
                if (afterCounter >= 0) {
                    //qDebug() << "Found potential counter before " << afterCounter << " in " << filename;

                    QStringList filenames = fileInfo.dir().entryList();
                    QString p;
                    QRegularExpression r;
                    while (true) {
                        p = QRegularExpression::escape(filename.left(afterCounter)).replace(QRegularExpression("[0-9]"),"[0-9]");
                        r = QRegularExpression("^"+p, QRegularExpression::CaseInsensitiveOption);
                        int countMatches = 0;
                        QSet<QString> uniqueNumbers;
                        for (int i = 0; i < filenames.size(); i++) {
                            QString fn = filenames[i];
                            if (MovieFile::isMovieFile(fn) && fn.indexOf(r) >= 0) {
                                countMatches++;
                                uniqueNumbers.insert(fn.mid(matchedCounter.capturedStart(),matchedCounter.capturedLength()));
                            }
                        }
                        //qDebug() << "nb match " << countMatches << " over " << filenames.size() << " files";
                        //qDebug() << "nb unique " << uniqueNumbers.size() << uniqueNumbers ;
                        if (uniqueNumbers.size() > 4 ) {
                            nbCharsToTrim = afterCounter;
                            afterCounter = pattern()["subSectionSeparator"].match(filename,afterCounter+1).capturedEnd();
                            //qDebug() << afterCounter;
                            if (afterCounter <= nbCharsToTrim) {
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                } else {
                    break;
                }
            }
            title = extractTitle(filename.mid(nbCharsToTrim));
        }
    }

    return title;
}

/* Constraint: the returned string must always be an exact substring of the filename */
QString MovieFile::extractTitle(QString filename) {
    QStringList sections = filename.split(pattern()["sectionSeparator"], QString::SkipEmptyParts);
    if (sections.size() == 0) {
        return filename;
    }

    ////qDebug() << filename;
    QString result = sections.at(0);
    if (sections.size()>1 &&
            filename.indexOf(QRegularExpression("^\\[[^\\]]*\\][^\\[]{2}")) >= 0) {
        result = sections.at(1);
    }
    //qDebug() << "section:" << result;

    result = result.mid(pattern()["startingSeparators"].match(result).capturedEnd());//remove starting seperators
    //qDebug() << "startin:" <<result;

    result = result.left(getLastYearOffset(result));
    //qDebug() << "year   :" <<result;

    int indexOfFirstNonWord = result.indexOf(pattern()["nonWord"]);
    result = result.left(indexOfFirstNonWord);
    result = result.left(result.indexOf(pattern()["endingSeparators"]));//remove trailing seperators
    //qDebug() << "nonword:" <<result;

    //remove trailing word that is likely not part of the title
    result = result.left(result.indexOf(pattern()["likelyEndingWordNotPartOfTitle"],4));
    //qDebug() << "endword:" <<result;

    QStringList subsections = result.split(pattern()["subSectionSeparator"]);
    int titlesize = subsections.at(0).size();
    int cumulatedSize = titlesize;
    //qDebug() << subsections;
    ////qDebug() << pattern()["likelyNonTitle"];
    for (int i = 1; i<subsections.size(); i++) {
        cumulatedSize += 3 + subsections.at(i).size();
        if (subsections.at(i).indexOf(pattern()["likelyNonTitle"])>=0) {
            //qDebug() << "subsect:" <<i << "likelyNonTitle";
            break;
        }
        int goodTitleSize = 55;
        if (abs(cumulatedSize-goodTitleSize) < abs(titlesize-goodTitleSize)) {
            titlesize = cumulatedSize;
        }
    }
    result = result.left(titlesize);

    //Arrete.moi.si.tu.peux.de.Steven.Spielberg.avec.L\u00C3\u00A9onardo.di.Caprio-Tom.Hanks
    //                     ^^^^^^^^^^
    result = result.left(result.indexOf(pattern()["castPreview"],20));


    result = result.left(result.indexOf(pattern()["endingSeparators"]));//remove trailing seperators

    if (result.count(pattern()["lowercaseChar"]) > 3){
        //if the string contains lowercase characters
        //remove the last word if it is fully uppercase
        result = result.left(result.indexOf(pattern()["endingUppercaseWord"]));
    }

    result = result.left(result.indexOf(pattern()["endingSeparators"]));//remove trailing seperators
    return result;
}

QString MovieFile::extractYear() {
    QString filename = fileInfo.completeBaseName();
    QString year = "";
    int yearIdx = getLastYearOffset(filename);
    if (yearIdx >= 0) {
        year = filename.mid(yearIdx,4);
    }
    return year;
}

int MovieFile::getLastYearOffset(QString s) {
    QRegularExpressionMatchIterator i = pattern()["year"].globalMatch(s,1);
    int offset = -1;
    while (i.hasNext()) {
        QRegularExpressionMatch m = i.next();
        offset = m.capturedStart();
    }
    return offset;
}

Movie *MovieFile::getMovie() {
    if (movie != NULL) {
        return movie;
    } else {
        if (!searchQuerySent) {
            searchQuerySent = true;
            TMDBQuery *tmdbquery = TMDBQuery::newSearchQuery(getTitle());
            connect(tmdbquery, SIGNAL(response(QJsonDocument)), this, SLOT(handleSearchResults(QJsonDocument)));
            tmdbquery->send(true);
        }
    }
    return NULL;
}

void MovieFile::handleSearchResults(QJsonDocument doc) {
    //qDebug() << "handleSearchResults(doc)";
    searchQuerySent = false;
    QJsonValue results = doc.object()["results"];
    if (results.isArray()) {
        QJsonArray array = results.toArray();
        if (array.size() > 0) {
            int movieID = array[0].toObject()["id"].toInt(-1);
            //qDebug() << "MovieID=" << movieID;
            if (movieID >= 0) {
                movie = Movie::getMovie(movieID);
                emit hasChanged(row);
                connect(movie, SIGNAL(hasChanged()), this, SLOT(movieHasChanged()));
            }
        } else {
            qWarning() << "empty results array";
        }
    } else {
        qWarning() << "results is not an array";
    }
}


QString MovieFile::computePrettyName() {
    QString richtext = "<html><head/><body><p>";

    QString filename = fileInfo.absoluteFilePath();

    if (titleSubString.size() > 0) {
        filename = filename.replace(titleSubString,"<span style=\" color:#03c010;\">"+titleSubString+"</span>");
    }
    if (year.size() > 0) {
        filename = filename.replace(year,"<span style=\" color:#2e0bd9;\">"+year+"</span>");
    }
    richtext += filename + "</p></body></html>";
    ////qDebug() << richtext;
    return richtext;
}

bool MovieFile::isMovieFile(QString filename) {
    return videoExtensions.contains(filename.split('.').last(), Qt::CaseInsensitive);
}

void MovieFile::movieHasChanged() {
    //qDebug() << "movieHasChanged()";
    //qDebug() << QString("emit hasChanged(%1)").arg(row);
    emit hasChanged(row);
}
