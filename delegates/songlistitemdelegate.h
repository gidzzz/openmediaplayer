#ifndef SONGLISTITEMDELEGATE_H
#define SONGLISTITEMDELEGATE_H

#include <QObject>
#include <QPainter>
#include <QRect>
#include <QModelIndex>
#include <QString>
#include <QFont>
#include <QColor>
#include <QStyledItemDelegate>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QApplication>
#include <QStyleOptionViewItem>
#include "includes.h"

class SongListItemDelegate : public QStyledItemDelegate
{
public:
        explicit SongListItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~SongListItemDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};


#endif // SONGLISTITEMDELEGATE_H
