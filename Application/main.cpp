#include "mainwindow.h"
#include <QApplication>
#include <qdebug.h>
#include <QElapsedTimer>

static bool debugMode = false;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!debugMode &&
            (type == QtInfoMsg || type == QtDebugMsg)) {
        return;
    }

    QString prefix;
    switch (type) {
    case QtInfoMsg:
        prefix = "Info    ";
        break;
    case QtDebugMsg:
        prefix = "Debug   ";
        break;
    case QtWarningMsg:
        prefix = "Warning ";
        break;
    case QtCriticalMsg:
        prefix = "Critical";
        break;
    case QtFatalMsg:
        prefix = "Fatal   ";
        break;
    }
    QTextStream(stderr) << QString("%1: %2 (%3:%4, %5)\n")
                     .arg(prefix)
                     .arg(msg)
                     .arg(context.file)
                     .arg(context.line)
                     .arg(context.function)
                     .toStdString().c_str();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageOutput);
    if (argc > 1 && QString(argv[1]) == "-d" || true) {
        debugMode = true;
    }

    qDebug() << "Hello World!!";

    VideoCollectionModel *movieModel = new VideoCollectionModel();
    MainWindow w(movieModel);
    w.show();

    movieModel->browseFolders();

    QFile file("tree.txt");
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);

    ModelNode::topLevelNode()->PrintTo(stream);

    return a.exec();
}
