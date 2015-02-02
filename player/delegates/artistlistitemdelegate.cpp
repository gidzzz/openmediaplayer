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
    QString artistName = index.data(Qt::DisplayRole).toString();

    QString albumSongCount;
    albumSongCount.append(tr("%n album(s)", "", index.data(UserRoleAlbumCount).toInt()));
    albumSongCount.append(", ");
    albumSongCount.append(tr("%n song(s)", "", index.data(UserRoleSongCount).toInt()));

    painter->save();
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected)
        QStyledItemDelegate::paint(painter, option, QModelIndex());

    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

    painter->drawPixmap(r.right()-70+3, r.top()+3, 64, 64,
                        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(64));

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
