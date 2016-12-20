#ifndef MOVIECOLLECTIONMODEL_H
#define MOVIECOLLECTIONMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QProgressDialog>
#include <QSet>
#include "videofile.h"

enum Columns {
    colTitle,
    colDate,
    colPath,
    nbColumns
};
const int htmlRole = 31;
const int rawDataRole = 32;


class VideoCollectionModel : public QAbstractItemModel {
    Q_OBJECT
public:
    VideoCollectionModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    void browseFolders();
private:
    QStringList movieFolders;
    QList<VideoFile*> videoFiles;

    QStringList additionalColumns;
    QSet<QString> columnsSet;
    bool resetOnGoing;

    int browsedDirsAndFiles;
    int discoveredDirsAndFiles;
    QProgressDialog browseProgress;
    void listRecursively(QString folder, QFileInfoList &videoList);
private slots:
    void rowChanged(ModelNode *node);
    void updateProgressBar();
};

#endif // MOVIECOLLECTIONMODEL_H
