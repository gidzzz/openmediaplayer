#ifndef ARTISTLISTITEMDELEGATE_H
#define ARTISTLISTITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include "includes.h"

class ArtistListItemDelegate : public QStyledItemDelegate
{
public:
    explicit ArtistListItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
    virtual ~ArtistListItemDelegate() {}

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ARTISTLISTITEMDELEGATE_H
