#include "profiler.h"
#include <QDebug>

void Profiler::funcBegin(const QString &id) {
    stats[id].funcBegin();
}

void Profiler::funcEnd(const QString &id) {
    statsPrintTimer.start();
    stats[id].funcEnd();
}

Profiler::Profiler() {
    statsPrintTimer.setInterval(1000);
    statsPrintTimer.setSingleShot(true);
    connect(&statsPrintTimer, SIGNAL(timeout()), this, SLOT(printStats()));
}

void Profiler::printStats() {
    QMapIterator<QString,FuncStats> i(stats);
    while (i.hasNext()) {
        i.next();
        qDebug() << i.key() << "\t: " << i.value().nbCalls << " calls  \t" << i.value().elapsed << "msecs";
    }
}

Profiler *profiler() {
    static Profiler *p = NULL;
    if (p == NULL) {
        p = new Profiler();
    }
    return p;
}

void FuncStats::funcBegin() {
    nbCalls++;
    timer.start();
}

void FuncStats::funcEnd() {
    elapsed += timer.elapsed();
}

FuncStats::FuncStats() {
    nbCalls = 0;
    elapsed = 0;
}
