#ifndef MAINDELEGATE_H
#define MAINDELEGATE_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QModelIndex>
#include <QString>
#include <QFont>
#include <QColor>
#include <QStyledItemDelegate>
#include "includes.h"

#include <QMaemo5Style>

class MainDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MainDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // MAINDELEGATE_H
