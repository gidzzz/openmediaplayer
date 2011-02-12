#include "artistlistitemdelegate.h"

void ArtistListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString artistName = index.data(UserRoleSongName).toString();
    //QString albumSongCount = "1 album, 1 song";
    QString albumSongCount;
    QPixmap albumArt;
    albumSongCount.append(index.data(UserRoleAlbumCount).toString() + " ");
    if(index.data(UserRoleAlbumCount).toInt() != 1)
        albumSongCount.append(tr("albums"));
    else
        albumSongCount.append(tr("album"));
    albumSongCount.append(", ");
    albumSongCount.append(index.data(UserRoleSongCount).toString() + " ");
    if(index.data(UserRoleSongCount).toInt() != 0)
        albumSongCount.append(tr("songs"));
    else
        albumSongCount.append(tr("song"));
    if(!index.data(UserRoleAlbumArt).isNull())
        albumArt = QPixmap(index.data(UserRoleAlbumArt).toString());
    else
        albumArt = QPixmap(defaultAlbumArt);


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
        painter->drawText(30, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, artistName, &r);

        r = option.rect;
        f.setPointSize(13);
        painter->setFont(f);
        r.setBottom(r.bottom()-10);
        painter->setPen(QPen(gray));
        painter->drawText(30, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, albumSongCount, &r);
        painter->setPen(defaultPen);;

        r = option.rect;
        painter->drawPixmap(r.right()-70, r.top()+4, 64, 64, albumArt);

    } else {
        // Portrait
        r = option.rect;
        f.setPointSize(18);
        painter->setFont(f);
        painter->drawText(r.left()+5, r.top()+5, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, artistName, &r);

        r = option.rect;
        f.setPointSize(13);
        painter->setFont(f);
        r.setBottom(r.bottom()-10);
        painter->setPen(QPen(gray));
        painter->drawText(r.left()+5, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, albumSongCount, &r);
        painter->setPen(defaultPen);;
        r = option.rect;
        painter->drawPixmap(r.right()-(64+13), r.top()+4, 64, 64, albumArt);
    }
    painter->restore();
}

QSize ArtistListItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(400, 70);
}
