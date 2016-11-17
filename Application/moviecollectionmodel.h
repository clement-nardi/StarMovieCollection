#ifndef MOVIECOLLECTIONMODEL_H
#define MOVIECOLLECTIONMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QProgressDialog>
#include "moviefile.h"

const int colID = 0;
const int colTitle = 1;
const int colYear = 2;
const int colPath = 3;


class MovieCollectionModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MovieCollectionModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void browseFolders();
private:
    QStringList movieFolders;
    QList<MovieFile*> moviefiles;

    int browsedDirsAndFiles;
    int discoveredDirsAndFiles;
    QProgressDialog browseProgress;
    void listRecursively(QString folder, QFileInfoList &videoList);
private slots:
    void rowChanged(int row);
    void updateProgressBar();
};

#endif // MOVIECOLLECTIONMODEL_H
