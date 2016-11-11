#include "moviefile.h"
#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QStringList>
#include <QJsonArray>

MovieFile::MovieFile(QString path, QObject *parent) : QObject(parent) {
    fileInfo = QFileInfo(path);
}


class CommonPatterns: public QMap<QString,QRegularExpression> {
public:
    CommonPatterns();

    QStringList treatSubJsonObject(QJsonObject obj) {
        QStringList result;
        if (!obj["listFromFile"].isNull() || !obj["listFromFile"].isUndefined()) {
            QString filename = obj["listFromFile"].toString();
            QFile file(filename);
            if (file.exists() && file.open(QIODevice::ReadOnly)) {
                result = QString(file.readAll()).replace("\r", "").replace('\n', ' ').split(' ', QString::SkipEmptyParts);
            } else {
                qWarning() << "Missing file: " << file.fileName();
            }
        } else {
            qWarning() << "Unsuported Format, expected \"listFromFile\" instead of: " << obj;
        }
        return result;
    }
};

CommonPatterns::CommonPatterns(){
    QFile file(QCoreApplication::applicationDirPath() + "/titlePatterns.json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            QMap<QString,QStringList> patterns;
            QJsonObject obj = doc.object();
            QStringList keys = obj.keys();
            for (int i = 0; i < keys.size(); i++) {
                QString key = keys.at(i);
                if (obj[key].isString()) {
                    patterns[key] << obj[key].toString();
                } else if (obj[key].isArray()) {
                    QJsonArray array = obj[key].toArray();
                    for (int j = 0; j < array.size(); j++) {
                        if (array[j].isString()) {
                            patterns[key] << array[j].toString();
                        } else if (array[j].isObject()) {
                            patterns[key] << treatSubJsonObject(array[j].toObject());
                        }
                    }
                } else if (obj[key].isObject()) {
                    patterns[key] << treatSubJsonObject(obj[key].toObject());
                }
            }


            QMap<QString,QString> pattern;
            for (int i = 0; i < keys.size(); i++) {
                QString key = keys[i];
                pattern[key] = QString("(%1)").arg(patterns[key].join('|'));
            }
            //find inter-references like %sep
            for (int d = 0; d < 3; d++) {
                for (int i = 0; i < keys.size(); i++) {
                    QString key1 = keys[i];
                    for (int j = 0; j < keys.size(); j++) {
                        if (i != j) {
                            QString key2 = keys[j];
                            pattern[key2].replace("%"+key1, pattern[key1]);
                        }
                    }
                }
            }
            for (int i = 0; i < keys.size(); i++) {
                this->insert(keys[i],QRegularExpression(pattern[keys[i]],QRegularExpression::CaseInsensitiveOption));
            }

        } else {
            qWarning() << "Expected a JSON Object in titlePatterns.json";
        }
    } else {
        qWarning() << "Missing file: " << file.fileName();
    }
    qDebug() << *this;
}

static CommonPatterns pattern() {
    static CommonPatterns p;
    return p;
}


/* Constraint: the returned string must always be an exact substring of the filename */
QString MovieFile::extractTitleFromFilename() {
    QString filename = fileInfo.completeBaseName();

    QStringList sections = filename.split(pattern()["sectionSeparator"], QString::SkipEmptyParts);
    if (sections.size() == 0) {
        return filename;
    }

    QString result = sections.at(0);
    if (sections.size()>1 &&
            filename.indexOf(QRegularExpression("^\\[[^\\]]*\\][^\\[]{2}")) >= 0) {
        result = sections.at(1);
    }

    int indexOfFirstNonWord = result.indexOf(pattern()["nonWord"],
                                                     1); //in case the title starts with a 4-digit year

    result = result.left(indexOfFirstNonWord);
    result = result.left(result.indexOf(pattern()["endingSeparators"]));//remove trailing seperators
    //qDebug() << result;

    //remove trailing word that is likely not part of the title
    result = result.left(result.indexOf(pattern()["likelyEndingWordNotPartOfTitle"],1));
    //qDebug() << result;

    QStringList subsections = result.split(pattern()["subSectionSeparator"]);
    int titlesize = subsections.at(0).size();
    int cumulatedSize = titlesize;
    //qDebug() << subsections;
    //qDebug() << pattern()["likelyNonTitle"];
    for (int i = 1; i<subsections.size(); i++) {
        cumulatedSize += 3 + subsections.at(i).size();
        if (subsections.at(i).indexOf(pattern()["likelyNonTitle"])>=0) {
            //qDebug() << i << "likelyNonTitle";
            break;
        }
        int goodTitleSize = 40;
        if (abs(cumulatedSize-goodTitleSize) < abs(titlesize-goodTitleSize)) {
            titlesize = cumulatedSize;
        }
    }
    result = result.left(titlesize);

    //Arrete.moi.si.tu.peux.de.Steven.Spielberg.avec.L\u00C3\u00A9onardo.di.Caprio-Tom.Hanks
    //                     ^^^^^^^^^^
    result = result.left(result.indexOf(pattern()["castPreview"],20));


    result = result.left(result.indexOf(pattern()["endingSeparators"]));//remove trailing seperators
    return result;
}

QString MovieFile::extractAlternateTitle() {
    return "";
}

QString MovieFile::getYear() {
    QString filename = fileInfo.completeBaseName();
    QString year = "";

    int yearIdx = filename.indexOf(QRegularExpression("(19|20|21)[0-9]{2}"),
                                   1); //in case the title starts with a 4-digit year
    if (yearIdx >= 0) {
        year = filename.mid(yearIdx,4);
    }
    return year;
}

QString MovieFile::prettyName() {
    QString title = extractTitleFromFilename();
    QString year = getYear();
    QString richtext = "<html><head/><body><p>";

    QString filename = fileInfo.completeBaseName();

    if (title.size() > 0) {
        filename = filename.replace(title,"<span style=\" color:#03c010;\">"+title+"</span>");
    }
    if (year.size() > 0) {
        filename = filename.replace(year,"<span style=\" color:#2e0bd9;\">"+year+"</span>");
    }
    richtext += filename + "</p></body></html>";
    //qDebug() << richtext;
    return richtext;
}

bool MovieFile::isMovieFile(QString filename) {
    return videoExtensions.contains(filename.split('.').last(), Qt::CaseInsensitive);
}
