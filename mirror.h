#ifndef MIRROR_H
#define MIRROR_H

#include <QGraphicsRectItem>
#include <QGraphicsItem>
#include <QPainter>
#include <QLinearGradient>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QTransform>

class mirror : public QGraphicsRectItem {

public:
    mirror(): mItem(0){
    }

    void setItem(QGraphicsItem* item){
        mItem = item;
        this->setRect(item->boundingRect().translated(0, item->boundingRect().height()));
        this->setParentItem(mItem);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0){
        if (!mItem && mItem->isVisible())
            return;

        QSize buffSize = option->rect.size(); //+ QSize(1,1);

        // Option
        QStyleOptionGraphicsItem opt = *option;
        opt.rect = QRect(QPoint(0,0), buffSize);

        // Buffer
        QImage buff(buffSize, QImage::Format_RGB32);
        QPainter p(&buff);

        //Draw into the buffer
        p.fillRect(opt.rect, QBrush(Qt::transparent));

        // Paint the item in the image starting from 0,0
        p.save();
        qreal dx = mItem->boundingRect().x();
        qreal dy = 0.9;
        p.translate(-dx, -dy);
        mItem->paint(&p, &opt, widget);
        p.restore();

        QLinearGradient g(0,0,0,buffSize.height());
        g.setColorAt(1, Qt::transparent);
        g.setColorAt(0.8, widget->palette().background().color());
        p.fillRect(opt.rect, g);

        // Draw buffer on the screen
        painter->setOpacity(0.5);
        painter->drawImage(option->rect.adjusted(0,0,1,1), buff.mirrored());//, opt.rect);
    }

private:
    QGraphicsItem *mItem;
};
#endif // MIRROR_H
