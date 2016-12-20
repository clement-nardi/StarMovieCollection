#ifndef PROFILER_H
#define PROFILER_H

#include <QMap>
#include <QString>
#include <QElapsedTimer>
#include <QTimer>
#include <QObject>

class FuncStats {
public:
    void funcBegin();
    void funcEnd();
    FuncStats();
    qint64 elapsed;
    int nbCalls;
private:
    QElapsedTimer timer;
};

class Profiler: public QObject {
    Q_OBJECT
public:
    void funcBegin(const QString &id);
    void funcEnd(const QString &id);
    friend Profiler *profiler();
private:
    Profiler();
    QMap<QString,FuncStats> stats;
    QTimer statsPrintTimer;
private slots:
    void printStats();
};

Profiler *profiler();



#endif // PROFILER_H
