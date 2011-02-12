#ifndef INTERNETRADIODELEGATE_H
#define INTERNETRADIODELEGATE_H

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

class InternetRadioDelegate : public QStyledItemDelegate
{
public:
        explicit InternetRadioDelegate(QObject *parent=0) : QStyledItemDelegate(parent) {}
        virtual ~InternetRadioDelegate() {}

        void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
#endif // INTERNETRADIODELEGATE_H
