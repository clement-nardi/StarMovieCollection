#include "tests.h"
#include "tmdbapi.h"
#include <QObject>
#include <QSignalSpy>

void Tests::TMDBSearch() {
    QBENCHMARK {
        TMDBQuery *tmdbquery;
        QSignalSpy *spy;
        tmdbquery = TMDBQuery::NewSearchQuery("les évadés");
        spy = new QSignalSpy(tmdbquery,SIGNAL(Response(QJsonDocument)));
        tmdbquery->Send();
        while (spy->count() == 0) {
            QTest::qWait(1);
        }
        QCOMPARE(spy->count(), 1);
        QList<QVariant> arguments = spy->takeFirst(); // take the first signal
        //qDebug() << arguments.at(0).toString();
        QJsonDocument response = arguments.at(0).toJsonDocument();
        QVERIFY(response.isObject());
        QJsonObject obj = response.object();
        QVERIFY(obj["page"].toInt() == 1);
        QVERIFY(obj["results"].isArray());
        QCOMPARE(obj["results"].toArray()[2].toObject()["original_title"].toString(), QString("The Shawshank Redemption"));
        QCOMPARE(obj["results"].toArray()[2].toObject()["id"].toInt(), 278);
        tmdbquery->deleteLater();
        spy->deleteLater();
    }
}

void Tests::TMDBMovie() {
    QBENCHMARK {
        TMDBQuery *tmdbquery;
        QSignalSpy *spy;
        tmdbquery = TMDBQuery::NewMovieQuery(278);
        spy = new QSignalSpy(tmdbquery,SIGNAL(Response(QJsonDocument)));
        tmdbquery->Send();
        while (spy->count() == 0) {
            QTest::qWait(1);
        }
        QList<QVariant> arguments = spy->takeFirst(); // take the first signal
        QJsonDocument response = arguments.at(0).toJsonDocument();
        QVERIFY(response.isObject());
        QJsonObject obj = response.object();
        QCOMPARE(obj["imdb_id"].toString(), QString("tt0111161"));
        QCOMPARE(obj["original_title"].toString(), QString("The Shawshank Redemption"));
        tmdbquery->deleteLater();
        spy->deleteLater();
    }

}

QTEST_MAIN(Tests)
