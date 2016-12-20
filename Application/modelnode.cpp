#include "modelnode.h"
#include <QSet>
#include <QDebug>

ModelNode::ModelNode(ModelNode *parent_)
    :data(){
    //qDebug() << QString("new ModelNode(%1)").arg((qulonglong)parent_);
    tmdbQuery = NULL;
    parentNode = NULL;
    setParentNode(parent_);
}

void ModelNode::setParentNode(ModelNode *node) {
    if (parentNode != NULL) {
        parentNode->children.removeOne(this);
    }
    parentNode = node;
    if (parentNode != NULL) {
        parentNode->children.append(this);
    }
}

ModelNode *ModelNode::getParentNode() {
    return parentNode;
}

int ModelNode::nbChildren()
{
    return children.size();
}

ModelNode *ModelNode::childAt(int i) {
    return children.at(i);
}

int ModelNode::getPositionAmongSiblings() {
    ModelNode *parent = getParentNode();
    if (parent != NULL) {
        return parent->children.indexOf(this);
    } else {
        return 0;
    }
}

QString ModelNode::getTitle() {
    if (tmdbQuery != NULL) {
        tmdbQuery->send(true);
    }
    return getTitleText();
}

QString ModelNode::getTitleText() {
    return "";
}

QString ModelNode::getDate() {
    return "";
}

QString ModelNode::getPath() {
    return "";
}

QString ModelNode::getHtmlTitle(){
    return "";
}

QString ModelNode::getHtmlDate(){
    return "";
}

QString ModelNode::getHtmlPath(){
    return "";
}

ModelNode *ModelNode::topLevelNode() {
    //qDebug() << "topLevelNode()";
    static ModelNode *node = NULL;
    if (node == NULL) {
        node = new ModelNode(NULL);
    }
    return node;
}

void ModelNode::PrintTo(QTextStream &io, int indent) {
    io << QString("%1%2 (%3 children)\n")
          .arg("",indent,QLatin1Char(' '))
          .arg(getTitleText())
          .arg(nbChildren());
    for (int i = 0; i < nbChildren(); i++) {
        childAt(i)->PrintTo(io,indent+3);
    }
}

void ModelNode::setTMDBQuery(TMDBQuery *query) {
    tmdbQuery = query;
    connect(tmdbQuery, SIGNAL(response(QJsonDocument)), this, SLOT(handleResults(QJsonDocument)),Qt::QueuedConnection);
    tmdbQuery->send();
}

void ModelNode::handleResults(QJsonDocument doc) {
    //qDebug() << QString("handleMovieResults(doc)");
    data = doc.object();
    delete tmdbQuery;
    tmdbQuery = NULL;
    emit hasChanged(this);
}
