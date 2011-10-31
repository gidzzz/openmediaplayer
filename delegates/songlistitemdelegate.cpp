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

#include "songlistitemdelegate.h"

void SongListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(Qt::DisplayRole).toString();

    painter->save();
    QRect r = option.rect;

    if (index.data(Qt::UserRole).toBool()) {
#ifdef Q_WS_MAEMO_5
        QColor activeColor = QMaemo5Style::standardColor("ActiveTextColor");
#else
        QColor activeColor(0,255, 0);
#endif
        painter->setPen(QPen(activeColor));
        painter->drawText(r, Qt::AlignVCenter|Qt::AlignCenter, title);
    }

    else  {
        int duration = index.data(UserRoleSongDuration).toInt();
        QString songLength = duration == Duration::Blank ? "" :
                             duration == Duration::Unknown ? "--:--" :
                                         time_mmss(duration);

        QString valueText;
        if (!index.data(UserRoleSongArtist).toString().isEmpty())
            valueText = index.data(UserRoleSongArtist).toString()
                      + " / "
                      + index.data(UserRoleSongAlbum).toString();
        else
            valueText = index.data(UserRoleValueText).toString();


        if (option.state & QStyle::State_Selected)
            QStyledItemDelegate::paint(painter, option, QModelIndex());

#ifdef Q_WS_MAEMO_5
        QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
        QColor secondaryColor(156, 154, 156);
#endif

        QFont f = painter->font();

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

        titleWidth = titleWidth - 15 - 15; // left and right margin

        QFontMetrics fm1(f);
        title = fm1.elidedText(title, Qt::ElideRight, titleWidth);
        painter->drawText(15, r.top()+5, titleWidth, r.height(), Qt::AlignTop|Qt::AlignLeft, title);

        f.setPointSize(13);
        painter->setFont(f);
        painter->setPen(QPen(secondaryColor));
        r.setBottom(r.bottom()-10);

        QFontMetrics fm2(f);
        valueText = fm2.elidedText(valueText, Qt::ElideRight, titleWidth);
        painter->drawText(15, r.top(), titleWidth, r.height(), Qt::AlignBottom|Qt::AlignLeft, valueText);
    }

    painter->restore();
}

QSize SongListItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
