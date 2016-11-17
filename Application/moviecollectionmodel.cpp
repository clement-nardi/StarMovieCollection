#include "moviecollectionmodel.h"

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QElapsedTimer>
#include <QTimer>

void MovieCollectionModel::listRecursively(QString folder, QFileInfoList &videoList) {
    QFileInfoList list = QDir(folder).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList dirList;
    browsedDirsAndFiles++;
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).isDir()) {
            dirList << list.at(i);
            discoveredDirsAndFiles++;
        } else {
            if (MovieFile::isMovieFile(list.at(i).absoluteFilePath())) {
                videoList << list.at(i);
                discoveredDirsAndFiles++;
            }
        }
    }
    for (int i = 0; i < dirList.size(); i++) {
        listRecursively(dirList.at(i).absoluteFilePath(), videoList);
    }
}

MovieCollectionModel::MovieCollectionModel() {    
    movieFolders << "W:/Videos/Films de Science-Fiction/"
                 << "W:/Videos/Films Français/"
                 << "W:/Videos/Films Américains/"
                 << "W:/Videos/Films d'autres nationalités/"
                 << "W:/Videos/Films Documentaires/"
                 << "W:/Videos/Films historiques et biopics/"
                 << "W:/Videos/Films Japonais/"
                 << "W:/Videos/Films pas encore vus/"
                 << "W:/Videos/Films pour enfants/"
                 << "W:/Videos/Séries/"
                 << "W:/Videos/Spectacles (Comiques, Théatre)/";
}

void MovieCollectionModel::updateProgressBar() {
    browseProgress.setMaximum(discoveredDirsAndFiles);
    browseProgress.setValue(browsedDirsAndFiles);
    browseProgress.setLabelText(QString("browsing %1/%2").arg(browsedDirsAndFiles).arg(discoveredDirsAndFiles));
    //qDebug() << "updateProgressBar()" << browsedDirsAndFiles << "/" << discoveredDirsAndFiles;
}

void MovieCollectionModel::browseFolders() {
    QElapsedTimer t;
    t.start();
    beginResetModel();
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
        MovieFile *mf = new MovieFile(path,i);
        browsedDirsAndFiles++;
        moviefiles.append(mf);
        connect(mf,SIGNAL(hasChanged(int)),this,SLOT(rowChanged(int)));
        QApplication::processEvents();
    }
    pt.stop();
    browseProgress.hide();
    endResetModel();
    qWarning() << QString("Took %1.%2s to browse all files").arg(t.elapsed()/1000).arg((qint64)(t.elapsed()%1000),3,10,QLatin1Char('0'));
}


int MovieCollectionModel::rowCount(const QModelIndex & parent) const {
    return moviefiles.size();
}
int MovieCollectionModel::columnCount(const QModelIndex & parent) const {
    return 4;
}
QVariant MovieCollectionModel::data(const QModelIndex & index, int role) const {
    MovieFile *mf = moviefiles[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case colID:
            if (mf->getMovie()!=NULL) {
                return mf->getMovie()->id;
            } else {
                return "";
            }
        case colTitle:
            return mf->getTitle();
        case colYear:
            return mf->year;
        case colPath:
            return mf->prettyName;
        }
    }
    return QVariant();
}
QVariant MovieCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
            }
        }
    }
    return QVariant();
}

void MovieCollectionModel::rowChanged(int row) {
    //qDebug() << QString("rowChanged(%1)").arg(row);
    emit dataChanged(createIndex(row,0),
                     createIndex(row,columnCount()-1));
}
