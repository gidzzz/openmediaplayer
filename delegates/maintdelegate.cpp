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

#include "delegates/maintdelegate.h"

void MainDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(Qt::DisplayRole).toString();
    QString subtitle = index.data(UserRoleValueText).toString();

    painter->save();
    QRect r = option.rect;

    if (option.state & QStyle::State_Selected)
        QStyledItemDelegate::paint(painter, option, QModelIndex());

#ifdef Q_WS_MAEMO_5
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif

    QFont f = painter->font();

    painter->drawPixmap(r.left(), r.top(), 164, 164,
                        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(164));

    f.setPointSize(18);
    painter->setFont(f);

    if (!subtitle.isEmpty()) {
        painter->drawText(170, r.top(), r.width(), r.height()-82, Qt::AlignBottom|Qt::AlignLeft, title);

        f.setPointSize(13);
        painter->setFont(f);
        painter->setPen(QPen(secondaryColor));
        painter->drawText(170, r.top()+86, r.width(), r.height(), Qt::AlignTop|Qt::AlignLeft, subtitle);
    }
    else
        painter->drawText(170, r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, title);

    painter->restore();
}

QSize MainDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 164);
}
