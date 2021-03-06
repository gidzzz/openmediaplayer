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

#include "playlistdelegate.h"

void PlayListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(UserRoleSongTitle).toString();

    QString songArtistAlbum;
    if (!index.data(UserRoleSongArtist).toString().isEmpty())
        songArtistAlbum = index.data(UserRoleSongArtist).toString()
                        + " / "
                        + index.data(UserRoleSongAlbum).toString();
    else
        songArtistAlbum = " ";

    QString songLength = mmss_len(index.data(UserRoleSongDuration).toInt());

    painter->save();
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected)
        QStyledItemDelegate::paint(painter, option, QModelIndex());

    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

    QFont f = painter->font();
    QPen defaultPen = painter->pen();

    f.setPointSize(18);
    painter->setFont(f);

    int titleWidth;
    if (!songLength.isEmpty()) {
        r.setRight(r.right()-12);
        painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);
        titleWidth = r.left();
        r = option.rect;
    } else
        titleWidth = r.width();

    titleWidth = titleWidth - 10 - 15; // left and right margin

    f.setPointSize(18);
    painter->setFont(f);

    QFontMetrics fm1(f);
    title = fm1.elidedText(title, Qt::ElideRight, titleWidth);
    painter->drawText(10, r.top()+5, titleWidth, r.height(), Qt::AlignTop|Qt::AlignLeft, title);

    f.setPointSize(13);
    painter->setFont(f);
    painter->setPen(QPen(secondaryColor));
    r.setBottom(r.bottom()-10);

    QFontMetrics fm2(f);
    songArtistAlbum = fm2.elidedText(songArtistAlbum, Qt::ElideRight, titleWidth);
    painter->drawText(10, r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, songArtistAlbum);

    painter->restore();
}

QSize PlayListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
