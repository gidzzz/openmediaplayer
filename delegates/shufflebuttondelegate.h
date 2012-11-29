#ifndef SHUFFLEBUTTONDELEGATE_H
#define SHUFFLEBUTTONDELEGATE_H

#include <QObject>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QMaemo5ValueButton>
#include <QListView>
#include <QEvent>
#include "includes.h"

class ShuffleButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ShuffleButtonDelegate(QListView *parent = 0);
    virtual ~ShuffleButtonDelegate();

    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QMaemo5ValueButton *button;

private slots:
    void onActivated(QModelIndex index);
};


#endif // SHUFFLEBUTTONDELEGATE_H
