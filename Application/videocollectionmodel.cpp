#include "videocollectionmodel.h"
#include "patterns.h"

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QElapsedTimer>
#include <QTimer>

void VideoCollectionModel::listRecursively(QString folder, QFileInfoList &videoList) {
    QFileInfoList list = QDir(folder).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList dirList;
    browsedDirsAndFiles++;
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).isDir()) {
            dirList << list.at(i);
            discoveredDirsAndFiles++;
        } else {
            if (VideoFile::isVideoFile(list.at(i).absoluteFilePath())) {
                videoList << list.at(i);
                discoveredDirsAndFiles++;
            }
        }
    }
    for (int i = 0; i < dirList.size(); i++) {
        listRecursively(dirList.at(i).absoluteFilePath(), videoList);
    }
}

VideoCollectionModel::VideoCollectionModel() {
    movieFolders << "W:/Videos/Films de Science-Fiction/"
                 /*<< "W:/Videos/Films Français/"
                 << "W:/Videos/Films Américains/"
                 << "W:/Videos/Films d'autres nationalités/"
                 << "W:/Videos/Films Documentaires/"
                 << "W:/Videos/Films historiques et biopics/"
                 << "W:/Videos/Films Japonais/"
                 << "W:/Videos/Films pas encore vus/"
                 << "W:/Videos/Films pour enfants/"
                 << "W:/Videos/Séries/"*/
                 << "W:/Videos/Spectacles (Comiques, Théatre)/";
    resetOnGoing = false;
}

void VideoCollectionModel::updateProgressBar() {
    browseProgress.setMaximum(discoveredDirsAndFiles);
    browseProgress.setValue(browsedDirsAndFiles);
    browseProgress.setLabelText(QString("browsing %1/%2").arg(browsedDirsAndFiles).arg(discoveredDirsAndFiles));
    //qDebug() << "updateProgressBar()" << browsedDirsAndFiles << "/" << discoveredDirsAndFiles;
}

void VideoCollectionModel::browseFolders() {
    QElapsedTimer t;
    t.start();
    beginResetModel();
    resetOnGoing = true;
    browsedDirsAndFiles = 0;
    discoveredDirsAndFiles = 0;
    QTimer pt;
    pt.setInterval(100);
    pt.setSingleShot(false);
    connect(&pt, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    pt.start();
    browseProgress.show();
    QFileInfoList videoList;
    for (int i = 0; i < movieFolders.size(); i++) {
        listRecursively(movieFolders.at(i), videoList);
    }
    for (int i = 0; i < videoList.size(); i++) {
        QString path = videoList.at(i).absoluteFilePath();
        VideoFile *mf = new VideoFile(path,i);
        browsedDirsAndFiles++;
        moviefiles.append(mf);
        connect(mf,SIGNAL(hasChanged(int)),this,SLOT(rowChanged(int)));
        QApplication::processEvents();
    }
    pt.stop();
    browseProgress.hide();
    endResetModel();
    resetOnGoing = false;
    qWarning() << QString("Took %1.%2s to browse all files").arg(t.elapsed()/1000).arg((qint64)(t.elapsed()%1000),3,10,QLatin1Char('0'));
}


int VideoCollectionModel::rowCount(const QModelIndex & parent) const {
    return moviefiles.size();
}
int VideoCollectionModel::columnCount(const QModelIndex & parent) const {
    return nbColumns + additionalColumns.size();
}

QVariant VideoCollectionModel::data(const QModelIndex & index, int role) const {
    VideoFile *mf = moviefiles[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case colID:
            if (mf->getVideo()!=NULL) {
                return mf->getVideo()->id;
            } else {
                return "";
            }
        case colTitle:
            return mf->getTitle();
        case colYear:
            return mf->year;
        case colPath:
            return mf->fileInfo.absoluteFilePath();
        default:
            if (index.column() >= nbColumns) {
                Video *movie = mf->getVideo();
                if (movie != NULL) {
                    return movie->data[additionalColumns[index.column() - nbColumns]].toVariant();
                }
            }
        }
        break;
    case htmlRole:
        switch (index.column()) {
        case colPath:
        {
            QString filename = mf->fileInfo.absoluteFilePath();
            QString richtext;
            if (mf->titleSubString.size() > 0) {
                filename = filename.replace(mf->titleSubString,"<span style=\" color:#03c010;\">"+mf->titleSubString+"</span>");
            }
            if (mf->year.size() > 0) {
                filename = filename.replace(mf->year,"<span style=\" color:#2e0bd9;\">"+mf->year+"</span>");
            }
            richtext += filename;
            ////qDebug() << richtext;
            return richtext;
        }
        default:
            if (index.column() >= nbColumns) {
                QString text = data(index,Qt::DisplayRole).toString();
                if (text.size() > 0) {
                    QString columnTitle = headerData(index.column(),Qt::Horizontal).toString();
                    if (columnTitle == "release_date" && text.size() > 4 && mf->year.size()>0) {
                        int releaseYear = text.left(4).toInt();
                        int fileYear = mf->year.toInt();
                        int yearDistance = abs(releaseYear - fileYear);
                        QString color;
                        switch (yearDistance) {
                        case 0: color = "2e0bd9"; break;
                        case 1: color = "3399aa"; break;
                        case 2: color = "f19855"; break;
                        default: color = "ff1111";
                        }
                        return QString("<span style=\" color:#%1;\">%2</span>%3")
                                .arg(color)
                                .arg(text.left(4))
                                .arg(text.mid(4));
                    } else if (columnTitle == "title" || columnTitle == "original_title") {
                        QSet<QString> wordsToMatch = QSet<QString>::fromList(text.split(pattern()["anySep"],QString::SkipEmptyParts));

                        if (wordsToMatch.size()>0) {
                            QSetIterator<QString> wi(wordsToMatch);
                            bool matched = false;
                            while (wi.hasNext()) {
                                QString word = wi.next();
                                QRegularExpression wordRegexp = QRegularExpression("(^|[^a-z])" +
                                                                                   QRegularExpression::escape(word) +
                                                                                   "($|[^a-z])",
                                                                                   QRegularExpression::CaseInsensitiveOption);
                                if (wordRegexp.match(mf->titleSubString).hasMatch()) {
                                    text.replace(word,"<span style=\" color:#2e0bd9;\">" + word + "</span>");
                                    matched = true;
                                } else if (wordRegexp.match(mf->fileInfo.absoluteFilePath()).hasMatch()) {
                                    text.replace(word,"<span style=\" color:#3399aa;\">" + word + "</span>");
                                    matched = true;
                                }
                            }
                            if (matched) {
                                return text;
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    return QVariant();
}
QVariant VideoCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            switch (section) {
            case colTitle:
                return "Title";
            case colYear:
                return "Year";
            case colPath:
                return "Path";
            case colID:
                return "id";
            default:
                if (section >= nbColumns) {
                    return additionalColumns[section-nbColumns];
                }
            }
        }
    }
    return QVariant();
}

void VideoCollectionModel::rowChanged(int row) {
    //qDebug() << QString("rowChanged(%1)").arg(row);
    Video *movie = moviefiles.at(row)->getVideo();
    if (movie != NULL) {
        QStringList keys = movie->data.keys();
        //qDebug() << "keys: " << keys;
        QStringList headersToAppend;
        for (int i = 0; i < keys.size(); i++) {
            QString key = keys.at(i);
            if (!columnsSet.contains(key)) {
                columnsSet.insert(key);
                headersToAppend << key;
            }
        }
        if (headersToAppend.size() > 0) {
            //qDebug() << "inserting " << headersToAppend.size() << " columns";
            if (!resetOnGoing)
                beginInsertColumns(QModelIndex(),columnCount(),columnCount()+headersToAppend.size()-1);
            additionalColumns << headersToAppend;
            if (!resetOnGoing)
                endInsertColumns();
        }
    }

    if (!resetOnGoing) {
        emit dataChanged(createIndex(row,0),
                         createIndex(row,columnCount()-1));
    }
}
