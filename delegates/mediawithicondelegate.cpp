#include "mediawithicondelegate.h"

void MediaWithIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(UserRoleTitle).toString();

    painter->save();
    QRect r = option.rect;

    if (!index.data(UserRoleObjectID).toBool()) {
#ifdef Q_WS_MAEMO_5
        QColor activeColor = QMaemo5Style::standardColor("ActiveTextColor");
#else
        QColor activeColor(0,255, 0);
#endif
        painter->setPen(QPen(activeColor));
        painter->drawText(r, Qt::AlignVCenter|Qt::AlignCenter, title);
    }

    else {
        QString songLength;
        int duration = index.data(UserRoleSongDuration).toInt();
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

        if (option.state & QStyle::State_Selected) {
    #ifdef Q_WS_MAEMO_5
            painter->drawImage(r, QImage("/etc/hildon/theme/images/TouchListBackgroundPressed.png"));
    #else
            painter->fillRect(r, option.palette.highlight().color());
    #endif
        }

        QFont f = painter->font();
        QFontMetrics fm(f);
        QPen defaultPen = painter->pen();

        int titleWidth;
        if (!songLength.isEmpty()) {
            r.setRight(r.right()-12);
            painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);
            titleWidth = r.left();
            r = option.rect;
        } else {
            titleWidth = r.width();
        }

        titleWidth = titleWidth - (15+48+15) - 15;
        title = fm.elidedText(title, Qt::ElideRight, titleWidth);
        painter->drawText(15+48+15, r.top(), titleWidth, r.height(), Qt::AlignVCenter|Qt::AlignLeft, title);

        if (index.data(Qt::DecorationRole).type() == QVariant::Icon) {
            QPixmap icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(48);
            painter->drawPixmap(15, r.top()+11, 48, 48, icon);
        }
    }

    painter->restore();
}

QSize MediaWithIconDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(400, 70);
}
