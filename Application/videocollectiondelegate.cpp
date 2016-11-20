#include "videocollectiondelegate.h"
#include <QDebug>

#include "videocollectionmodel.h"
#include <QLabel>
#include <QPainter>

#include <QTextDocument>


VideoCollectionDelegate::VideoCollectionDelegate()
{

}

void VideoCollectionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    //qDebug() << QString("paint(%1,%2)").arg(index.row()).arg(index.column());
    QVariant html = index.data(htmlRole);
    if (html.isValid()) {
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
        doc.setHtml(html.toString());

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
