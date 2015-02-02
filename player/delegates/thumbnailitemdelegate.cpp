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
    QString title = index.data(UserRoleTitle).toString();
    QString description = index.data(UserRoleValueText).toString();

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected)
        QStyledItemDelegate::paint(painter, option, QModelIndex());

    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");

    painter->drawPixmap(r.x()+(r.width()-128)/2, r.y()+3, 128, 128,
                        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(128, 128));

    int margin = ( r.width() - (10+128+10) )/2;
    r.setLeft(r.left()+margin);
    r.setRight(r.right()-margin);

    QFontMetrics fm(painter->font());

    title = fm.elidedText(title, Qt::ElideRight, r.width());
    painter->drawText(r.x(), r.top()+134, r.width(), r.height(), Qt::AlignHCenter, title);

    painter->setPen(QPen(secondaryColor));

    description = fm.elidedText(description, Qt::ElideRight, r.width());
    painter->drawText(r.x(), r.top()+142+painter->font().pointSize(), r.width(), r.height(), Qt::AlignHCenter, description);

    painter->restore();
}

QSize ThumbnailItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(155, 180);
}
