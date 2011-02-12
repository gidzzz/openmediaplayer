#include "internetradiodelegate.h"

void InternetRadioDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        QString songName = index.data(UserRoleSongTitle).toString();
        QString URI = index.data(UserRoleSongURI).toString();

        painter->save();
        QRect r = option.rect;
        if(option.state & QStyle::State_Selected)
        {
            r = option.rect;
#ifdef Q_WS_MAEMO_5
            painter->drawImage(r, QImage("/etc/hildon/theme/images/TouchListBackgroundPressed.png"));
#else
            painter->fillRect(r, option.palette.highlight().color());
#endif
        }
        QFont f = painter->font();
        QPen defaultPen = painter->pen();
        QColor gray;
        gray = QColor(156, 154, 156);

        r = option.rect;
        f.setPointSize(18);
        painter->setFont(f);
        painter->drawText(r.left()+5, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

        r = option.rect;
        f.setPointSize(13);
        painter->setFont(f);
        r.setBottom(r.bottom()-10);
        painter->setPen(QPen(gray));
        painter->drawText(r.left()+5, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, URI, &r);
        painter->setPen(defaultPen);;

        painter->restore();
}

QSize InternetRadioDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
