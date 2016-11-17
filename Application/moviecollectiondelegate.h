#ifndef MOVIECOLLECTIONDELEGATE_H
#define MOVIECOLLECTIONDELEGATE_H

#include <QStyledItemDelegate>

class MovieCollectionDelegate : public QStyledItemDelegate
{
public:
    MovieCollectionDelegate();


    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

#endif // MOVIECOLLECTIONDELEGATE_H
