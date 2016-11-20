#include "tests.h"
#include "tmdbapi.h"
#include <QObject>
#include <QSignalSpy>
#include "videofile.h"
#include "romanconverter.h"

void Tests::testRomanConverter() {

    QMap<int,QString> m;
    m[1] = "I";
    m[2] = "II";
    m[3] = "III";
    m[4] = "IV";
    m[5] = "V";
    m[6] = "VI";
    m[7] = "VII";
    m[8] = "VIII";
    m[9] = "IX";
    m[10] = "X";
    m[11] = "XI";
    m[12] = "XII";
    m[13] = "XIII";
    m[14] = "XIV";
    m[15] = "XV";
    m[35] = "XXXV";
    m[1985] = "MCMLXXXV";

    QMapIterator<int,QString> i(m);
    while (i.hasNext()) {
        i.next();
        QCOMPARE(i.key(),roman2int(i.value()));
        QCOMPARE(int2roman(i.key()),i.value());
    }
}

void Tests::testTMDBSearch() {
    QBENCHMARK {
        TMDBQuery *tmdbquery;
        QSignalSpy *spy;
        tmdbquery = TMDBQuery::newSearchQuery("les évadés");
        spy = new QSignalSpy(tmdbquery,SIGNAL(response(QJsonDocument)));
        tmdbquery->send();
        while (spy->count() == 0) {
            QTest::qWait(1);
        }
        QCOMPARE(spy->count(), 1);
        QList<QVariant> arguments = spy->takeFirst(); // take the first signal
        //qDebug() << arguments.at(0).toString();
        QJsonDocument doc = arguments.at(0).toJsonDocument();
        QVERIFY(doc.isObject());
        QJsonObject obj = doc.object();
        QVERIFY(obj["page"].toInt() == 1);
        QVERIFY(obj["results"].isArray());
        QCOMPARE(obj["results"].toArray()[2].toObject()["original_title"].toString(), QString("The Shawshank Redemption"));
        QCOMPARE(obj["results"].toArray()[2].toObject()["id"].toInt(), 278);
        tmdbquery->deleteLater();
        spy->deleteLater();
    }
    QCOMPARE(TMDBQuery::getCacheHitRatio(), 1.0);
}

void Tests::testTMDBMovie() {
    QBENCHMARK {
        TMDBQuery *tmdbquery;
        QSignalSpy *spy;
        tmdbquery = TMDBQuery::newMovieQuery(278);
        spy = new QSignalSpy(tmdbquery,SIGNAL(response(QJsonDocument)));
        tmdbquery->send();
        while (spy->count() == 0) {
            QTest::qWait(1);
        }
        QList<QVariant> arguments = spy->takeFirst(); // take the first signal
        QJsonDocument doc = arguments.at(0).toJsonDocument();
        QVERIFY(doc.isObject());
        QJsonObject obj = doc.object();
        QCOMPARE(obj["imdb_id"].toString(), QString("tt0111161"));
        QCOMPARE(obj["original_title"].toString(), QString("The Shawshank Redemption"));
        tmdbquery->deleteLater();
        spy->deleteLater();
    }

    QCOMPARE(TMDBQuery::getCacheHitRatio(), 1.0);
}

void Tests::testIsVideoFile() {
    QVERIFY(VideoFile::isVideoFile(QString("toto/tata.avi")));
    QVERIFY(VideoFile::isVideoFile(QString("toto/tata.mkv")));
    QVERIFY(VideoFile::isVideoFile(QString("toto/tata.MkV")));
    QVERIFY(VideoFile::isVideoFile(QString("toto/tata.txt.avi")));
    QVERIFY(!VideoFile::isVideoFile(QString("toto/tata.avi.txt")));
    QVERIFY(!VideoFile::isVideoFile(QString("tata")));
}

void Tests::testExtractTitle(){

    QFile file("../../Tests/movieTitleTests.txt");
    QVERIFY(file.exists());
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QVERIFY(!stream.atEnd());

    int countTitles = 0;
    int countMatches = 0;
    QStringList failedPaths;
    QStringList expectedTitles;

    while (!stream.atEnd()) {
        QString path = "../../Tests/" + stream.readLine();
        QString title = stream.readLine();
        countTitles++;
        //qDebug() << path;
        if (VideoFile(path,0,false).titleSubString == QString(title)) {
            countMatches++;
        } else {
            failedPaths << path;
            expectedTitles << title;
        }
    }

    for (int i = 0; i < failedPaths.size(); i++) {
        qDebug() << "Actual   :" << VideoFile(failedPaths[i],0,false).titleSubString;
        qDebug() << "Expected :" << expectedTitles[i];
    }

    QCOMPARE(countMatches, countTitles);
}

void Tests::testExtractSeasonEpisode() {
    QFile file("../../Tests/seasonEpisodeTests.txt");
    QVERIFY(file.exists());
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QVERIFY(!stream.atEnd());

    int countPaths = 0;
    int countMatches = 0;
    QStringList failedPaths;
    QList<int> expectedSeason;
    QList<int> expectedEpisode;

    while (!stream.atEnd()) {
        QString path = "../../Tests/" + stream.readLine();
        int seasonNumber = stream.readLine().toInt();
        int episodeNumber = stream.readLine().toInt();
        countPaths++;
        VideoFile v(path,0,false);
        //qDebug() << path;
        if (v.seasonNumber == seasonNumber && v.episodeNumber == episodeNumber) {
            countMatches++;
        } else {
            failedPaths     << path;
            expectedSeason  << seasonNumber;
            expectedEpisode << episodeNumber;
        }
    }

    for (int i = 0; i < failedPaths.size(); i++) {
        VideoFile v(failedPaths[i],0,false);
        qDebug() << "Actual   : season " << v.seasonNumber << " episode " << v.episodeNumber;
        qDebug() << "Expected : season " << expectedSeason[i] << " episode " << expectedEpisode[i];
    }

    QCOMPARE(countMatches, countPaths);
}

QTEST_MAIN(Tests)
