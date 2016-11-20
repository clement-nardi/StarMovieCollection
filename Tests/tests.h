
#ifndef TESTS_H
#define TESTS_H

#include <QtTest/QtTest>

class Tests : public QObject
{
    Q_OBJECT
private slots:

    void testRomanConverter();

    void testTMDBSearch();
    void testTMDBMovie();

    void testIsVideoFile();
    void testExtractSeasonEpisode();
    void testExtractTitle();

};

#endif // TESTS_H
