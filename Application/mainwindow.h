#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "moviecollectionmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(MovieCollectionModel *movieModel_, QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MovieCollectionModel *movieModel;
};

#endif // MAINWINDOW_H
