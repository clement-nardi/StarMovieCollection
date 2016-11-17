#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "moviecollectiondelegate.h"
#include "moviecollectionmodel.h"
#include <QSortFilterProxyModel>


MainWindow::MainWindow(MovieCollectionModel *movieModel_, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->movieCollectionTable->setItemDelegate(new MovieCollectionDelegate());
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);

    movieModel = movieModel_;
    proxyModel->setSourceModel(movieModel);
    ui->movieCollectionTable->setSortingEnabled(true);
    ui->movieCollectionTable->setModel(proxyModel);
}

MainWindow::~MainWindow()
{
    delete ui;
}
