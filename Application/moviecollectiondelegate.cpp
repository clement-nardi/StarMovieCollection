#include "moviecollectiondelegate.h"
#include <QDebug>

#include "moviecollectionmodel.h"
#include <QLabel>
#include <QPainter>

#include <QTextDocument>


MovieCollectionDelegate::MovieCollectionDelegate()
{

}

void MovieCollectionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    //qDebug() << QString("paint(%1,%2)").arg(index.row()).arg(index.column());
    if (index.column() == colPath) {
        QStyleOptionViewItemV4 options = option;
        initStyleOption(&options, index);
        painter->save();

        /*QPoint topLeft = option.rect.topLeft();
        QPoint bottomRight = option.rect.topRight();
        QLinearGradient backgroundGradient(topLeft, bottomRight);
        backgroundGradient.setColorAt(0.0, QColor(Qt::yellow));
        backgroundGradient.setColorAt(1.0, Qt::white);
        painter->fillRect(option.rect, QBrush(backgroundGradient));*/
        //painter->drawText(option.rect,0,index.data().toString());

        QTextDocument doc;
        doc.setHtml(index.data().toString());

        options.text = "";
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

        int docHeight = int(doc.size().height());
        int hShift = options.rect.top();
        if (options.rect.height() > docHeight) {
            //v-align
            hShift += (options.rect.height()-docHeight)/2;
        }

        painter->translate(options.rect.left(), hShift);
        QRect clip(0, 0, options.rect.width(), options.rect.height());
        doc.drawContents(painter,clip);

        painter->restore();
        return;
    } else {
        return QStyledItemDelegate::paint(painter, option, index);
    }
}
