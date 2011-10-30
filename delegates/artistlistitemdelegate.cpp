/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "artistlistitemdelegate.h"

void ArtistListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString artistName = index.data(UserRoleTitle).toString();

    QString albumSongCount = index.data(UserRoleAlbumCount).toString() + " ";
    albumSongCount.append( (index.data(UserRoleAlbumCount).toInt() == 1) ? tr("album") : tr("albums") );
    albumSongCount.append(", ");
    albumSongCount.append(index.data(UserRoleSongCount).toString() + " ");
    albumSongCount.append( (index.data(UserRoleSongCount).toInt() == 1) ? tr("song") : tr("songs") );

    QPixmap albumArt = index.data(UserRoleAlbumArt).isNull() ? QIcon::fromTheme(defaultAlbumIcon).pixmap(64) :
                                                               QPixmap(index.data(UserRoleAlbumArt).toString());

    painter->save();
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected) {
#ifdef Q_WS_MAEMO_5
        painter->drawImage(r, QImage("/etc/hildon/theme/images/TouchListBackgroundPressed.png"));
#else
        painter->fillRect(r, option.palette.highlight().color());
#endif
    }

#ifdef Q_WS_MAEMO_5
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif

    painter->drawPixmap(r.right()-70+3, r.top()+3, 64, 64, albumArt);
    int textWidth = r.width() - (15+70+15);

    QFont f = painter->font();

    f.setPointSize(18);
    painter->setFont(f);
    QFontMetrics fm1(f);
    artistName = fm1.elidedText(artistName, Qt::ElideRight, textWidth);
    painter->drawText(15, r.top()+5, textWidth, r.height(), Qt::AlignTop|Qt::AlignLeft, artistName);

    r.setBottom(r.bottom()-10);
    f.setPointSize(13);
    painter->setFont(f);
    painter->setPen(QPen(secondaryColor));
    QFontMetrics fm2(f);
    artistName = fm2.elidedText(albumSongCount, Qt::ElideRight, textWidth);
    painter->drawText(15, r.top(), textWidth, r.height(), Qt::AlignBottom|Qt::AlignLeft, albumSongCount);

    painter->restore();
}

QSize ArtistListItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(400, 70);
}
