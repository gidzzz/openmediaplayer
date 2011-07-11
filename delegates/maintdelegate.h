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
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QApplication>
#include <QTime>
#include <QStyleOptionViewItem>
#include "includes.h"

class MainDelegate : public QStyledItemDelegate
{
public:
        explicit MainDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~MainDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // MAINDELEGATE_H
