#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "videocollectiondelegate.h"
#include "videocollectionmodel.h"
#include <QSortFilterProxyModel>
#include <QLabel>
#include <QPlainTextEdit>

MainWindow::MainWindow(VideoCollectionModel *movieModel_, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    showRawDataAction("Show Raw Data") {
    ui->setupUi(this);

    ui->movieCollectionTable->setItemDelegate(new VideoCollectionDelegate());
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);

    movieModel = movieModel_;
    proxyModel->setSourceModel(movieModel);
    ui->movieCollectionTable->setSortingEnabled(true);
    ui->movieCollectionTable->header()->setSectionsMovable(true);
    ui->movieCollectionTable->setModel(proxyModel);
    ui->movieCollectionTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(movieModel, SIGNAL(modelReset()), this, SLOT(rearangeColumns()));
    connect(movieModel, SIGNAL(columnsInserted(QModelIndex,int,int)), this, SLOT(rearangeColumns()));


    ui->movieCollectionTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->movieCollectionTable,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));

    contextMenu.addAction(&showRawDataAction);
    connect(&showRawDataAction, SIGNAL(triggered()), this, SLOT(showRawData()));


}

void MainWindow::showContextMenu(QPoint p) {
    contextMenu.popup(ui->movieCollectionTable->mapToGlobal(p));
}

void MainWindow::showRawData() {
    QModelIndexList indexes = ui->movieCollectionTable->selectionModel()->selectedIndexes();
    if (indexes.size() > 0) {
        QJsonObject obj = ui->movieCollectionTable->model()->data(indexes[0],rawDataRole).toJsonObject();
        QString s(QJsonDocument(obj).toJson());
        QPlainTextEdit *textView = new QPlainTextEdit(s);
        textView->setAttribute(Qt::WA_DeleteOnClose);
        textView->setGeometry(50,50,1000,700);
        textView->show();
    }
}

void MainWindow::rearangeColumns() {
    /*QStringList preferedColumns;
    preferedColumns << "original_title"
                    << "title"
                    << movieModel->headerData(colTitle,Qt::Horizontal).toString()
                    << "release_date"
                    << movieModel->headerData(colDate,Qt::Horizontal).toString()
                    << movieModel->headerData(colPath,Qt::Horizontal).toString();
    for (int i = preferedColumns.size()-1; i >= 0; i--) {
        QString columnTitle = preferedColumns[i];
        for (int j = 0; j < movieModel->columnCount(); j++) {
            if (movieModel->headerData(j,Qt::Horizontal).toString() == columnTitle) {
                QHeaderView *hv = ui->movieCollectionTable->header();
                hv->moveSection(hv->visualIndex(j),0);
            }
        }
    }*/
    ui->movieCollectionTable->header()->setResizeContentsPrecision(50);
    ui->movieCollectionTable->resizeColumnToContents(0);
    ui->movieCollectionTable->resizeColumnToContents(1);
    ui->movieCollectionTable->resizeColumnToContents(2);
    ui->movieCollectionTable->resizeColumnToContents(3);
    /*ui->movieCollectionTable->resizeColumnToContents(4);
    ui->movieCollectionTable->resizeColumnToContents(5);
    ui->movieCollectionTable->resizeColumnToContents(6);*/
    ui->movieCollectionTable->sortByColumn(0,Qt::AscendingOrder);
}

MainWindow::~MainWindow()
{
    delete ui;
}
