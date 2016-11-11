
#ifndef TESTS_H
#define TESTS_H

#include <QtTest/QtTest>

class Tests : public QObject
{
    Q_OBJECT
private slots:

    void testTMDBSearch();
    void testTMDBMovie();

    void testIsMoviefile();
    void testExtractTitle();

};

#endif // TESTS_H
