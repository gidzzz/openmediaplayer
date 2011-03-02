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
        // Thanks to hqh for fapman, this code is based on the list in it.
        QString songName = index.data(UserRoleSongTitle).toString();
        QString songLength = index.data(UserRoleSongDuration).toString();
        QString songArtistAlbum = index.data(UserRoleSongArtist).toString() + " / " + index.data(UserRoleSongAlbum).toString();

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
            painter->drawText(r.left(), r.top()+5, r.width()-90, r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

            r = option.rect;
            f.setPointSize(13);
            painter->setFont(f);
            r.setBottom(r.bottom()-10);
            painter->setPen(QPen(gray));
            painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft, songArtistAlbum, &r);
            painter->setPen(defaultPen);;

            r = option.rect;
            r.setRight(r.right()-12);
            r.setTop(r.top()+5);
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r, Qt::AlignTop|Qt::AlignRight, songLength, &r);
        } else {
            // Portrait
            r = option.rect;
            f.setPointSize(18);
            painter->setFont(f);
            painter->drawText(r.left()+5, r.top()+5, r.width()-90, r.height(), Qt::AlignTop|Qt::AlignLeft, songName, &r);

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
            r.setTop(r.top()+5);
            painter->setFont(f);
            painter->drawText(r, Qt::AlignTop|Qt::AlignRight, songLength, &r);

        }
        painter->restore();
}

QSize PlayListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
