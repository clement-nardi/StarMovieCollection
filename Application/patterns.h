#ifndef PATTERNS_H
#define PATTERNS_H

#include <QStringList>
#include <QRegularExpression>
#include <QMap>

class CommonPatterns: public QMap<QString,QRegularExpression> {
public:
    friend CommonPatterns &pattern();
private:
    CommonPatterns();
    QStringList treatSubJsonObject(QJsonObject obj);
};

CommonPatterns &pattern();

#endif // PATTERNS_H
