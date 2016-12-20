#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "videocollectionmodel.h"
#include <QMenu>
#include <QAction>

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
    void showContextMenu(QPoint);
    void showRawData();
private:
    Ui::MainWindow *ui;
    VideoCollectionModel *movieModel;

    QMenu contextMenu;
    QAction showRawDataAction;
};

#endif // MAINWINDOW_H
