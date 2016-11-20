#include "patterns.h"
#include <QFile>
#include <QSet>
#include <QJsonObject>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>

QStringList CommonPatterns::treatSubJsonObject(QJsonObject obj) {
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

CommonPatterns::CommonPatterns(){
    QFile file(QCoreApplication::applicationDirPath() + "/titlePatterns.json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            QMap<QString,QStringList> patterns;
            QSet<QString> caseSensitive;
            QJsonObject obj = doc.object();
            QStringList keys = obj.keys();
            for (int i = 0; i < keys.size(); i++) {
                QString key = keys.at(i);
                if (key == "patternsThatAreCaseSensitive") {
                    QJsonArray array = obj[key].toArray();
                    for (int j = 0; j < array.size(); j++) {
                        caseSensitive.insert(array[j].toString());
                    }
                } else {
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
                if (caseSensitive.contains(keys[i])) {
                    this->insert(keys[i],QRegularExpression(pattern[keys[i]]));
                } else {
                    this->insert(keys[i],QRegularExpression(pattern[keys[i]],QRegularExpression::CaseInsensitiveOption));
                }
            }

        } else {
            qWarning() << "Expected a JSON Object in titlePatterns.json";
        }
    } else {
        qWarning() << "Missing file: " << file.fileName();
    }
    //qDebug() << *this;
}

CommonPatterns &pattern() {
    static CommonPatterns p;
    return p;
}
