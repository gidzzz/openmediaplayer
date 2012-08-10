#include "shufflebuttondelegate.h"

ShuffleButtonDelegate::ShuffleButtonDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    button = new QMaemo5ValueButton();
    button->setText(tr("Shuffle songs"));
    button->setIcon(QIcon::fromTheme(defaultShuffleIcon));
    button->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    button->setCheckable(true);
}

void ShuffleButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect r = option.rect;
    button->setFixedWidth(r.width());
    button->setFixedHeight(r.height());
    button->setValueText(tr("%n song(s)", "", index.data(UserRoleSongCount).toInt()));
    button->setEnabled(option.state & QStyle::State_Enabled);
    button->setChecked(option.state & QStyle::State_Selected && button->isEnabled());

    QPixmap pixmap(button->size());
    button->render(&pixmap);
    painter->drawPixmap(r.left(), r.top(), r.width(), r.height(), pixmap);
}

QSize ShuffleButtonDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
