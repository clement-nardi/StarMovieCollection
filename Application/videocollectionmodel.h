#ifndef MOVIECOLLECTIONMODEL_H
#define MOVIECOLLECTIONMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QProgressDialog>
#include <QSet>
#include "videofile.h"

enum Columns {
    colID,
    colTitle,
    colYear,
    colPath,
    nbColumns
};
const int htmlRole = 31;


class VideoCollectionModel : public QAbstractTableModel {
    Q_OBJECT
public:
    VideoCollectionModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void browseFolders();
private:
    QStringList movieFolders;
    QList<VideoFile*> moviefiles;

    QStringList additionalColumns;
    QSet<QString> columnsSet;
    bool resetOnGoing;

    int browsedDirsAndFiles;
    int discoveredDirsAndFiles;
    QProgressDialog browseProgress;
    void listRecursively(QString folder, QFileInfoList &videoList);
private slots:
    void rowChanged(int row);
    void updateProgressBar();
};

#endif // MOVIECOLLECTIONMODEL_H
