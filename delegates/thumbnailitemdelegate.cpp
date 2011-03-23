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

#include "thumbnailitemdelegate.h"

void ThumbnailItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(Qt::DisplayRole).toString();
    QPixmap thumbnail = QIcon(index.data(Qt::DecorationRole).value<QIcon>()).pixmap(128, 128);
    QString duration = index.data(UserRoleValueText).toString();

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
    f.setPointSize(13);
    painter->setFont(f);
    QPen defaultPen = painter->pen();
    QColor gray;
    gray = QColor(156, 154, 156);

    r = option.rect;
    painter->drawPixmap(r.x() + ((r.width()/2)-(128/2)), r.y(), 128, 128, thumbnail);

    r = option.rect;
    r.setLeft(r.left()+10);
    r.setRight(r.right()-10);
    painter->drawText(r.x(), r.y()+133, r.width(), r.height(), Qt::AlignHCenter, title, &r);

    r = option.rect;
    painter->setPen(QPen(gray));
    painter->drawText(r.x(), r.y()+ (142 + painter->font().pointSize()), r.width(), r.height(), Qt::AlignHCenter, duration, &r);
    painter->setPen(defaultPen);

    painter->restore();
}

QSize ThumbnailItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(155, 180);
}
