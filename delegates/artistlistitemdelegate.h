#ifndef ARTISTLISTITEMDELEGATE_H
#define ARTISTLISTITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5Style>
#endif

class ArtistListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ArtistListItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ARTISTLISTITEMDELEGATE_H
