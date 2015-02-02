#ifndef MEDIAWITHICONDELEGATE_H
#define MEDIAWITHICONDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include "includes.h"

#include <QMaemo5Style>

class MediaWithIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MediaWithIconDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // MEDIAWITHICONDELEGATE_H
