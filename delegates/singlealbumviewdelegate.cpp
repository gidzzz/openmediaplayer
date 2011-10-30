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

#include "singlealbumviewdelegate.h"

void SingleAlbumViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(UserRoleSongTitle).toString();

    int duration = index.data(UserRoleSongDuration).toInt();
    QString songLength = duration == Duration::Blank ? "" :
                         duration == Duration::Unknown ? "--:--" :
                                     time_mmss(duration);

    painter->save();
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected) {
#ifdef Q_WS_MAEMO_5
        painter->drawImage(r, QImage("/etc/hildon/theme/images/TouchListBackgroundPressed.png"));
#else
        painter->fillRect(r, option.palette.highlight().color());
#endif
    }

    QFontMetrics fm(painter->font());

    int titleWidth;
    if (!songLength.isEmpty()) {
        r.setRight(r.right()-12);
        painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);
        titleWidth = r.left();
        r = option.rect;
    } else
        titleWidth = r.width();

    titleWidth = titleWidth - 15 - 15; // left and right margin
    title = fm.elidedText(title, Qt::ElideRight, titleWidth);
    painter->drawText(15, r.top(), titleWidth, r.height(), Qt::AlignVCenter|Qt::AlignLeft, title);

    painter->restore();
}

QSize SingleAlbumViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
