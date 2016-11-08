
#ifndef TESTS_H
#define TESTS_H

#include <QtTest/QtTest>

class Tests : public QObject
{
    Q_OBJECT
private slots:

    void TMDBSearch();
    void TMDBMovie();

};

#endif // TESTS_H
