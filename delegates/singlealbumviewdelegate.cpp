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
        // Thanks to hqh for fapman, this code is based on the list in it.
        QString songName = index.data(UserRoleSongTitle).toString();

        int duration = index.data(UserRoleSongDuration).toInt();
        QString songLength;
        switch (duration) {
            case Duration::Blank:
                songLength = "";
                break;
            case Duration::Unknown:
                songLength = "--:--";
                break;
            default:
                songLength = time_mmss(duration);
        }

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

        r = option.rect;
        f.setPointSize(18);
        QFontMetrics fm(f);
        painter->setFont(f);
        int pf = fm.width(songLength);
        songName = fm.elidedText(songName, Qt::ElideRight, r.width()-pf-40);

        painter->drawText(15, r.top(), r.width()-pf, r.height(), Qt::AlignVCenter|Qt::AlignLeft, songName, &r);

        r = option.rect;
        r.setRight(r.right()-12);
        f.setPointSize(18);
        painter->setFont(f);
        painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);

        painter->restore();

}

QSize SingleAlbumViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
