#include "songlistitemdelegate.h"

void SongListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        // Thanks to hqh for fapman, this code is based on the list in it.
        QString songName = index.data(UserRoleSongName).toString();
        QString songLength = "--:--";
        QString songArtistAlbum = "(unknown artist) / (unknown album)";

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

        if( QApplication::desktop()->width() > QApplication::desktop()->height() )
        {
            // Landscape
            r = option.rect;
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(30, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

            r = option.rect;
            f.setPointSize(13);
            painter->setFont(f);
            r.setBottom(r.bottom()-10);
            painter->setPen(QPen(gray));
            painter->drawText(30, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, songArtistAlbum, &r);
            painter->setPen(defaultPen);;

            r = option.rect;
            r.setRight(r.right()-12);
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);
        } else {
            // Portrait
            r = option.rect;
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r.left()+5, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

            r = option.rect;
            f.setPointSize(13);
            painter->setFont(f);
            r.setBottom(r.bottom()-10);
            painter->setPen(QPen(gray));
            painter->drawText(r.left()+5, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, songArtistAlbum, &r);
            painter->setPen(defaultPen);;

            r = option.rect;
            r.setRight(r.right()-12);
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);

        }
        painter->restore();
}

QSize SongListItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
