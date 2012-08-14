#include "providerlistitemdelegate.h"

void ProviderListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        QStyledItemDelegate::paint(painter, option, QModelIndex());

    QString name = index.data(Qt::DisplayRole).toString();
    QString description = index.data(UserRoleValueText).toString();

    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
    QFont f = painter->font();
    QRect r = option.rect;
    int isz = option.decorationSize.width();
    if (isz < 0) isz = 48;
    int textWidth = r.width() - (3+isz+3);

    painter->save();

    painter->drawPixmap(3, r.top()+(70-isz)/2, isz, isz,
                        QIcon::fromTheme(index.data(Qt::UserRole).toBool() ? "general_tickmark_checked" : "general_tickmark_unchecked").pixmap(isz));

    f.setPointSize(18);
    painter->setFont(f);

    QFontMetrics fm1(f);
    name = fm1.elidedText(name, Qt::ElideRight, textWidth);
    painter->drawText(3+isz+3, r.top()+5, textWidth, r.height(), Qt::AlignTop|Qt::AlignLeft, name);

    f.setPointSize(13);
    painter->setFont(f);
    painter->setPen(QPen(secondaryColor));
    r.setBottom(r.bottom()-10);

    QFontMetrics fm2(f);
    description = fm2.elidedText(description, Qt::ElideRight, textWidth);
    painter->drawText(3+isz+3, r.top(), textWidth, r.height(), Qt::AlignBottom|Qt::AlignLeft, description);

    painter->restore();
}

QSize ProviderListItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
