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

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5Style>
#endif

class PlayListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PlayListDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // PLAYLISTDELEGATE_H
