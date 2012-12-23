#ifndef PROVIDERLISTITEMDELEGATE_H
#define PROVIDERLISTITEMDELEGATE_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QModelIndex>
#include <QString>
#include <QFont>
#include <QColor>
#include <QStyledItemDelegate>
#include <QMaemo5Style>
#include "includes.h"

class ProviderListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ProviderListItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // PROVIDERLISTITEMDELEGATE_H
