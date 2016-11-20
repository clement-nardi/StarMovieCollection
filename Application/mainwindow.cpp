#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "videocollectiondelegate.h"
#include "videocollectionmodel.h"
#include <QSortFilterProxyModel>


MainWindow::MainWindow(VideoCollectionModel *movieModel_, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->movieCollectionTable->setItemDelegate(new VideoCollectionDelegate());
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);

    movieModel = movieModel_;
    proxyModel->setSourceModel(movieModel);
    ui->movieCollectionTable->setSortingEnabled(true);
    ui->movieCollectionTable->horizontalHeader()->setSectionsMovable(true);
    ui->movieCollectionTable->setModel(proxyModel);
    ui->movieCollectionTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(movieModel, SIGNAL(modelReset()), this, SLOT(rearangeColumns()));
    connect(movieModel, SIGNAL(columnsInserted(QModelIndex,int,int)), this, SLOT(rearangeColumns()));
}

void MainWindow::rearangeColumns() {
    QStringList preferedColumns;
    preferedColumns << "original_title"
                    << "title"
                    << movieModel->headerData(colTitle,Qt::Horizontal).toString()
                    << "release_date"
                    << movieModel->headerData(colYear,Qt::Horizontal).toString()
                    << movieModel->headerData(colPath,Qt::Horizontal).toString();
    for (int i = preferedColumns.size()-1; i >= 0; i--) {
        QString columnTitle = preferedColumns[i];
        for (int j = 0; j < movieModel->columnCount(); j++) {
            if (movieModel->headerData(j,Qt::Horizontal).toString() == columnTitle) {
                QHeaderView *hv = ui->movieCollectionTable->horizontalHeader();
                hv->moveSection(hv->visualIndex(j),0);
            }
        }
    }
    ui->movieCollectionTable->horizontalHeader()->setResizeContentsPrecision(50);
    ui->movieCollectionTable->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete ui;
}
