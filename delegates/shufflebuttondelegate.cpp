#include "shufflebuttondelegate.h"

ShuffleButtonDelegate::ShuffleButtonDelegate(QListView *parent) :
    QStyledItemDelegate(parent)
{
    button = new QMaemo5ValueButton();
    button->setText(tr("Shuffle songs"));
    button->setIcon(QIcon::fromTheme(defaultShuffleIcon));
    button->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    button->setCheckable(true);

    connect(parent, SIGNAL(activated(QModelIndex)), this, SLOT(onActivated(QModelIndex)));
}

void ShuffleButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect r = option.rect;
    button->setFixedWidth(r.width());
    button->setFixedHeight(r.height());
    button->setValueText(tr("%n song(s)", "", index.data(UserRoleSongCount).toInt()));
    button->setEnabled(option.state & QStyle::State_Enabled);
    button->setChecked(option.state & QStyle::State_Selected);

    QPixmap pixmap(button->size());
    button->render(&pixmap);
    painter->drawPixmap(r.left(), r.top(), r.width(), r.height(), pixmap);
}

void ShuffleButtonDelegate::onActivated(QModelIndex index)
{
    if (index.row() == 0)
        static_cast<QListView*>(this->parent())->clearSelection();
}

QSize ShuffleButtonDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 70);
}
