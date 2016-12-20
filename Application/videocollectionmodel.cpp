#include "videocollectionmodel.h"
#include "patterns.h"

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QElapsedTimer>
#include <QTimer>
#include "profiler.h"

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
        VideoFile *vf = new VideoFile(path);
        browsedDirsAndFiles++;
        videoFiles.append(vf);
        connect(vf,SIGNAL(hasChanged(ModelNode*)),this,SLOT(rowChanged(ModelNode*)));
        QApplication::processEvents();
    }
    pt.stop();
    browseProgress.hide();
    endResetModel();
    resetOnGoing = false;
    qWarning() << QString("Took %1.%2s to browse all %3 files and folders")
                  .arg(t.elapsed()/1000)
                  .arg((qint64)(t.elapsed()%1000),3,10,QLatin1Char('0'))
                  .arg(browsedDirsAndFiles);
}


int VideoCollectionModel::rowCount(const QModelIndex & parent) const {
    ModelNode * node;
    if (parent.isValid()) {
        node = (ModelNode *) parent.internalPointer();
    } else {
        node = ModelNode::topLevelNode();
    }
    return node->nbChildren();
}
int VideoCollectionModel::columnCount(const QModelIndex & parent) const {
    return nbColumns;
}

QModelIndex VideoCollectionModel::index(int row, int column, const QModelIndex &parent) const {
    ModelNode * node;
    if (parent.isValid()) {
        node = (ModelNode *) parent.internalPointer();
    } else {
        node = ModelNode::topLevelNode();
    }
    return createIndex(row, column, node->childAt(row));
}

QModelIndex VideoCollectionModel::parent(const QModelIndex &index) const {
    if (index.isValid() && index.column() == 0) {
        ModelNode *node = (ModelNode *) index.internalPointer();
        ModelNode *parent = node->getParentNode();
        if (parent != NULL) {
            return createIndex(parent->getPositionAmongSiblings(), index.column(), parent);
        }
    }
    return QModelIndex();
}


QVariant VideoCollectionModel::data(const QModelIndex & index, int role) const {
    ModelNode *node = (ModelNode*)index.internalPointer();
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case colTitle: return node->getTitle();
        case colDate : return node->getDate();
        case colPath : return node->getPath();
        }
        break;
    case htmlRole:
        switch (index.column()) {
        case colTitle: return node->getHtmlTitle();
        case colDate : return node->getHtmlDate();
        case colPath : return node->getHtmlPath();
        }
        break;
    case rawDataRole:
        qDebug() << rawDataRole;
        qDebug() << node->data.keys();
        return node->data;
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
            case colDate:
                return "Year";
            case colPath:
                return "Path";
            }
        }
    }
    return QVariant();
}

void VideoCollectionModel::rowChanged(ModelNode *node) {
    //qDebug() << QString("rowChanged(%1)").arg(row);
    int row = node->getPositionAmongSiblings();
    if (!resetOnGoing) {
        emit dataChanged(createIndex(row,0              ,node->getParentNode()),
                         createIndex(row,columnCount()-1,node->getParentNode()));
    }
}
