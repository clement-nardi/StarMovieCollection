#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include <QDir>
#include "moviefile.h"
#include <QLabel>


void listRecursively(QString folder, QFileInfoList *files) {
    QFileInfoList list = QDir(folder).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    files->append(list);
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).isDir()) {
            listRecursively(list.at(i).absoluteFilePath(), files);
        }
    }
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStringList movieFolders;
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
    QFileInfoList files;
    for (int i = 0; i < movieFolders.size(); i++) {
        listRecursively(movieFolders.at(i),&files);
    }
    QFile file("allMovieFilesAndTitles.txt");
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    for (int i = 0; i < files.size(); i++) {
        QString path = files.at(i).absoluteFilePath();
        if (MovieFile::isMovieFile(path)) {
            MovieFile mf(path);
            ui->scrollAreaWidgetContents->layout()->addWidget(new QLabel(mf.prettyName()));
            stream << path << '\n' << mf.extractTitleFromFilename() << '\n';
        }
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}
