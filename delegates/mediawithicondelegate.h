#ifndef MEDIAWITHICONDELEGATE_H
#define MEDIAWITHICONDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5Style>
#endif

class MediaWithIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MediaWithIconDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // MEDIAWITHICONDELEGATE_H
