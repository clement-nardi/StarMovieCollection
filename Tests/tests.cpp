#include "tests.h"
#include "tmdbapi.h"
#include <QObject>
#include <QSignalSpy>
#include "moviefile.h"

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

void Tests::testIsMoviefile() {
    QVERIFY(MovieFile::isMovieFile(QString("toto/tata.avi")));
    QVERIFY(MovieFile::isMovieFile(QString("toto/tata.mkv")));
    QVERIFY(MovieFile::isMovieFile(QString("toto/tata.MkV")));
    QVERIFY(MovieFile::isMovieFile(QString("toto/tata.txt.avi")));
    QVERIFY(!MovieFile::isMovieFile(QString("toto/tata.avi.txt")));
    QVERIFY(!MovieFile::isMovieFile(QString("tata")));
}

void Tests::testExtractTitle(){

    QFile file("../../Tests/movieTitleTests.txt");
    QVERIFY(file.exists());
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    QVERIFY(!stream.atEnd());

    while (!stream.atEnd()) {
        QString path = stream.readLine();
        QString title = stream.readLine();
        QCOMPARE(MovieFile(path).extractTitleFromFilename(),
                 QString(title));
    }
}

QTEST_MAIN(Tests)
