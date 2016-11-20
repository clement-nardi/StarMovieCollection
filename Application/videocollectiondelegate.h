#ifndef MOVIECOLLECTIONDELEGATE_H
#define MOVIECOLLECTIONDELEGATE_H

#include <QStyledItemDelegate>

class VideoCollectionDelegate : public QStyledItemDelegate
{
public:
    VideoCollectionDelegate();


    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

#endif // MOVIECOLLECTIONDELEGATE_H
