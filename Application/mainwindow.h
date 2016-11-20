#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "videocollectionmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(VideoCollectionModel *movieModel_, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void rearangeColumns();
private:
    Ui::MainWindow *ui;
    VideoCollectionModel *movieModel;
};

#endif // MAINWINDOW_H
