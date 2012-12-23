#ifndef SINGLEALBUMVIEWDELEGATE_H
#define SINGLEALBUMVIEWDELEGATE_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QModelIndex>
#include <QString>
#include <QFont>
#include <QColor>
#include <QStyledItemDelegate>
#include "includes.h"

class SingleAlbumViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SingleAlbumViewDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // SINGLEALBUMVIEWDELEGATE_H
