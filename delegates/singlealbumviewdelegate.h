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
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QApplication>
#include <QStyleOptionViewItem>
#include "includes.h"

class SingleAlbumViewDelegate : public QStyledItemDelegate
{
public:
        explicit SingleAlbumViewDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~SingleAlbumViewDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // SINGLEALBUMVIEWDELEGATE_H
