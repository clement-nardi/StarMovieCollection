#include "videofile.h"
#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QStringList>
#include <QJsonArray>
#include <QDir>
#include <QStringListIterator>
#include "tmdbapi.h"
#include "video.h"
#include "romanconverter.h"
#include "patterns.h"

VideoFile::VideoFile(QString path, int row_, bool searchDataNow, QObject *parent) : QObject(parent) {
    video = NULL;
    seasonNumber = 0;
    episodeNumber = 0;
    searchQuerySent = false;
    row = row_;
    fileInfo = QFileInfo(path);
    extractYear();
    extractInfoFromFilename();
    extractSeasonEpisodeFromFilename();

    if (searchDataNow) {
        TMDBQuery *tmdbquery = TMDBQuery::newSearchQuery(getTitle());
        connect(tmdbquery, SIGNAL(response(QJsonDocument)), this, SLOT(handleSearchResults(QJsonDocument)), Qt::QueuedConnection);
        tmdbquery->send(false);
    }
}


QString VideoFile::getTitle() {
    return QString(titleSubString).replace("."," ").replace("_"," ");
}


void VideoFile::extractSeasonEpisodeFromMatch(const QRegularExpressionMatch &match) {
    if (match.hasMatch()) {
        for (int i = 0; i < 10 ; i++) {
            QString season = match.captured(QString("seasonNumber%1").arg(i));
            QString episode = match.captured(QString("episodeNumber%1").arg(i));
            //qDebug() << match.captured() << " " << i << " season " << season << " episode " << episode;
            if (season.size() > 0) {
                if (pattern()["romanSeasonNumber"].match(season).hasMatch()) {
                    seasonNumber = roman2int(season);
                } else {
                    seasonNumber = season.toInt();
                }
            }
            if (episode.size() > 0) {
                episodeNumber = episode.toInt();
            }
        }
        //qDebug() << match.captured() << "-->season " << seasonNumber << " episode " << episodeNumber;
    }
}

void VideoFile::findCounterInFilename(QFileInfo fi, QString &counter, int &nbCharsToTrim) {
    QString filename = fi.completeBaseName();
    //qDebug() << filename;
    int afterCounter = 0;
    nbCharsToTrim = 0;
    while (true) {
        //detect counter
        //qDebug() << filename.mid(afterCounter);
        QRegularExpressionMatch matchedCounter = pattern()["counter"].match(filename,afterCounter==0?0:afterCounter-1);
        afterCounter = matchedCounter.capturedEnd();
        if (afterCounter >= 0) {
            //qDebug() << "Found potential counter before " << afterCounter << " in " << filename;

            QStringList filenames = fi.dir().entryList();
            //qDebug() << filenames;
            QString p;
            QRegularExpression r;
            while (true) {
                p = QRegularExpression::escape(filename.left(afterCounter)).replace(QRegularExpression("[0-9]"),"[0-9]");
                r = QRegularExpression("^"+p, QRegularExpression::CaseInsensitiveOption);
                int countMatches = 0;
                QSet<QString> uniqueNumbers;
                for (int i = 0; i < filenames.size(); i++) {
                    QString fn = filenames[i];
                    if ((VideoFile::isVideoFile(fn) || fi.isDir()) && fn.indexOf(r) >= 0) {
                        countMatches++;
                        uniqueNumbers.insert(fn.mid(matchedCounter.capturedStart(),matchedCounter.capturedLength()));
                    }
                }
                //qDebug() << "nb match " << countMatches << " over " << filenames.size() << " files";
                //qDebug() << "nb unique " << uniqueNumbers.size() << uniqueNumbers ;
                if (uniqueNumbers.size() > 4 ) {
                    nbCharsToTrim = afterCounter;
                    counter = matchedCounter.captured("counterValue");
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
}

void VideoFile::extractInfoFromFilename() {
    //qDebug() << fileInfo.absoluteFilePath();

    QString filename = fileInfo.completeBaseName();

    QDir parentFolder = fileInfo.dir();
    QRegularExpressionMatch seasonIdMatchInParentFolder;

    do {
        seasonIdMatchInParentFolder = pattern()["seasonID"].match(parentFolder.dirName());
    } while (!seasonIdMatchInParentFolder.hasMatch() && parentFolder.cdUp());

    int nbCharsToTrim;
    if (seasonIdMatchInParentFolder.hasMatch()) {
        QString counter;
        findCounterInFilename(QFileInfo(parentFolder.absolutePath()),counter,nbCharsToTrim);
        if (nbCharsToTrim > 0) {
            //qDebug() << counter << " " << seasonIdMatchInParentFolder.capturedStart() << " " << nbCharsToTrim ;
            seasonIdMatchInParentFolder = pattern()["seasonID"].match(parentFolder.dirName().mid(nbCharsToTrim));
        }
    }

    //check if it's organized like this: Series_title/season_ID/episode
    if (seasonIdMatchInParentFolder.capturedStart() == 0) {
        //directory named like: "Season 1 HD"
        //n-2 folder must contain the series title
        parentFolder.cdUp();
        titleSubString = extractTitle(parentFolder.dirName());
        isTvEpisode = true;
    } else if (seasonIdMatchInParentFolder.capturedStart() > 0) {
        // like "The Big Bang Theory Season 1"
        titleSubString = extractTitle(parentFolder.dirName().mid(nbCharsToTrim));
        isTvEpisode = true;
    } else {
        QRegularExpressionMatch sMatch = pattern()["seasonID"].match(filename);
        QRegularExpressionMatch eMatch = pattern()["episodeID"].match(filename);
        QRegularExpressionMatch seMatch = pattern()["completeEpisodeID"].match(filename);
        QRegularExpressionMatch seMatchMaybe = pattern()["maybeCompleteEpisodeID"].match(filename);
        isTvEpisode = false;

        //qDebug() << QString("s=%1 e=%2 se=%3 se3d=%4").arg(sMatch.captured()).arg(eMatch.captured()).arg(seMatch.captured()).arg(seMatchMaybe.captured());

        //Detect Series Episode
        if ((eMatch.hasMatch() && sMatch.hasMatch()) ||
            seMatch.hasMatch() ) {
            //don't look for counters, they're expected
            //qDebug() << "Episode of a Series";
            isTvEpisode = true;
        } else if (seMatchMaybe.hasMatch()) {
            //qDebug() << "Maybe Episode of a Series?";
            // 3-digit episode IDs are source of problem
            // for instance it must be matched in:
            //    Fais.Pas.Ci,.Fais.Pas.Ca.-.201.-.Les.bonnes.manieres   // season 2 episode 1
            // But not in:
            //    N°.019.-.1961.Walt.Disney.-.Les.101.Dalmatiens.avi
            //Solution: look at nearby video files to see if they look like series episodes

            QStringList filenames = fileInfo.dir().entryList();
            int countEpisodes = 0;
            int countMovieFiles = 0;
            for (int i = 0; i < filenames.size(); i++) {
                QString fn = filenames[i];
                if (VideoFile::isVideoFile(fn)) {
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
                isTvEpisode = true;
            } else {
                isTvEpisode = false;
            }
        }
        if (isTvEpisode) {
            //qDebug() << "Episode of a Series";
            titleSubString = extractTitle(filename);
            if (titleSubString.size() == 0) {
                //backup to folder/filename
                titleSubString = extractTitle(fileInfo.dir().dirName() + "/" + filename);
            }
        } else {
            //Movie file
            //qDebug() << "Movie File";
            nbCharsToTrim;
            QString counter;
            findCounterInFilename(fileInfo,counter,nbCharsToTrim);
            titleSubString = extractTitle(filename.mid(nbCharsToTrim));
        }
    }
}

void VideoFile::extractSeasonEpisodeFromFilename() {
    if (isTvEpisode) {
        extractSeasonEpisodeFromMatch(pattern()["completeEpisodeID"].match(fileInfo.absoluteFilePath()));
        if (seasonNumber == 0) {
            extractSeasonEpisodeFromMatch(pattern()["seasonID"].match(fileInfo.absoluteFilePath()));
        }
        if (episodeNumber == 0) {
            extractSeasonEpisodeFromMatch(pattern()["maybeCompleteEpisodeID"].match(fileInfo.absoluteFilePath()));
        }
        if (episodeNumber == 0) {
            extractSeasonEpisodeFromMatch(pattern()["episodeID"].match(fileInfo.absoluteFilePath()));
        }
        if (episodeNumber == 0) {
            //last resort
            int nbCharsToTrim;
            QString counter;
            findCounterInFilename(fileInfo,counter,nbCharsToTrim);
            if (counter.size() > 0) {
                episodeNumber = counter.toInt();
            }
        }
    }
}

/* Constraint: the returned string must always be an exact substring of the filename */
QString VideoFile::extractTitle(QString filename) {
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

void VideoFile::extractYear() {
    QString filename = fileInfo.completeBaseName();
    year = "";
    int yearIdx = getLastYearOffset(filename);
    if (yearIdx >= 0) {
        year = filename.mid(yearIdx,4);
    }
}

int VideoFile::getLastYearOffset(QString s) {
    QRegularExpressionMatchIterator i = pattern()["year"].globalMatch(s,1);
    int offset = -1;
    while (i.hasNext()) {
        QRegularExpressionMatch m = i.next();
        offset = m.capturedStart("year");
    }
    return offset;
}

Video *VideoFile::getVideo() {
    if (video != NULL) {
        return video;
    } else {
        if (!searchQuerySent) {
            searchQuerySent = true;
            TMDBQuery *tmdbquery = TMDBQuery::newSearchQuery(getTitle());
            connect(tmdbquery, SIGNAL(response(QJsonDocument)), this, SLOT(handleSearchResults(QJsonDocument)), Qt::QueuedConnection);
            tmdbquery->send(true);
        }
    }
    return NULL;
}

void VideoFile::handleSearchResults(QJsonDocument doc) {
    //qDebug() << "handleSearchResults(doc)";
    searchQuerySent = false;
    QJsonValue results = doc.object()["results"];
    if (results.isArray()) {
        QJsonArray array = results.toArray();
        if (array.size() > 0) {
            int bestMatch = 0;
            int bestIdx = 0;

            for (int i = 0; i < array.size(); i++) {
                int match = 0;
                if (array[i].isObject()) {
                    QJsonObject searchResult = array[i].toObject();
                    QStringList wordsToMatch;
                    wordsToMatch << searchResult["original_title"].toString().split(pattern()["anySep"],QString::SkipEmptyParts);
                    wordsToMatch << searchResult["title"].toString().split(pattern()["anySep"],QString::SkipEmptyParts);

                    if (wordsToMatch.size()>0) {
                        QStringListIterator wi(wordsToMatch);
                        int wordMatchPoints = 0;
                        while (wi.hasNext()) {
                            QString word = wi.next();
                            QRegularExpression wordRegexp = QRegularExpression("(^|[^a-z])" +
                                                                               QRegularExpression::escape(word) +
                                                                               "($|[^a-z])",
                                                                               QRegularExpression::CaseInsensitiveOption);
                            if (wordRegexp.match(titleSubString).hasMatch()) {
                                wordMatchPoints += 3;
                            } else if (wordRegexp.match(fileInfo.absoluteFilePath()).hasMatch()) {
                                wordMatchPoints += 2;
                            }
                        }
                        match += wordMatchPoints * 50 / (3*wordsToMatch.size());
                    }
                    QString releaseDate = searchResult["release_date"].toString();
                    if (year.size() > 0 && releaseDate.size() >= 4) {
                        int releaseYear = releaseDate.left(4).toInt();
                        int fileYear = year.toInt();
                        int yearDistance = abs(releaseYear - fileYear);
                        switch (yearDistance) {
                        case 0: match += 25;
                        case 1: match += 18;
                        case 2: match += 5;
                        }
                    }
                    QString type = searchResult["media_type"].toString();
                    if (type == "tv" && isTvEpisode ||
                        type == "movie" && (!isTvEpisode)) {
                        match += 25;
                    }
                    if (match > bestMatch) {
                        bestMatch = match;
                        bestIdx = i;
                    }
                }
            }

            int id = array[bestIdx].toObject()["id"].toInt(-1);
            QString type = array[bestIdx].toObject()["media_type"].toString();

            //qDebug() << "MovieID=" << movieID;
            if (id >= 0) {
                if (type == "movie") {
                    video = Video::getMovie(id);
                } else if (type == "tv") {
                    video = Video::getTVEpisode(id,seasonNumber,episodeNumber);
                }
                if (video != NULL) {
                    emit hasChanged(row);
                    connect(video, SIGNAL(hasChanged()), this, SLOT(movieHasChanged()));
                }
            }
        } else {
            qWarning() << "empty results array";
        }
    } else {
        qWarning() << "results is not an array";
    }
}

bool VideoFile::isVideoFile(QString filename) {
    return videoExtensions.contains(filename.split('.').last(), Qt::CaseInsensitive);
}

void VideoFile::movieHasChanged() {
    //qDebug() << "movieHasChanged()";
    //qDebug() << QString("emit hasChanged(%1)").arg(row);
    emit hasChanged(row);
}
