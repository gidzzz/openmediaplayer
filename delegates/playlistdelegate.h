#ifndef PLAYLISTDELEGATE_H
#define PLAYLISTDELEGATE_H

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
#include <QTime>
#include <QStyleOptionViewItem>
#include "includes.h"

class PlayListDelegate : public QStyledItemDelegate
{
public:
        explicit PlayListDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~PlayListDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // PLAYLISTDELEGATE_H
