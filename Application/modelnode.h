#ifndef VIDEOCOLLECTION_H
#define VIDEOCOLLECTION_H

#include <QString>
#include <QList>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include "tmdbapi.h"

class ModelNode: public QObject {
    Q_OBJECT
public:
    ModelNode(ModelNode *parent_ = topLevelNode());

    void setParentNode(ModelNode *);
    ModelNode *getParentNode();

    virtual int nbChildren();
    virtual ModelNode *childAt(int i);
    int getPositionAmongSiblings();

    QString getTitle();
    virtual QString getDate();
    virtual QString getPath();
    virtual QString getHtmlTitle();
    virtual QString getHtmlDate();
    virtual QString getHtmlPath();

    QJsonObject data;

    static ModelNode *topLevelNode();

    void PrintTo(QTextStream &io, int indent = 0);


    virtual QString getTitleText();

protected:
    void setTMDBQuery(TMDBQuery *query);
    TMDBQuery *tmdbQuery;

private:
    QList<ModelNode *> children;
    ModelNode *parentNode;

signals:
    void hasChanged(ModelNode *);

private slots:
    virtual void handleResults(QJsonDocument doc);
};

#endif // VIDEOCOLLECTION_H
