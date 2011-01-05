#ifndef NOWPLAYINGINDICATOR_H
#define NOWPLAYINGINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QTimer>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <mafwrendereradapter.h>
#include "ui_nowplayingindicator.h"
#include "includes.h"

namespace Ui {
    class NowPlayingIndicator;
}

class MafwRendererAdapter;

class NowPlayingIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit NowPlayingIndicator(QWidget *parent = 0);
    ~NowPlayingIndicator();

signals:
    void clicked();

private:
    Ui::NowPlayingIndicator *ui;
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent *);
    QList<QPixmap> images;
    QTimer *timer;
    QString indicatorImage;
#ifdef Q_WS_MAEMO_5
    MafwRendererAdapter* mafwrenderer;
#endif
    int frame;

private slots:
    void onStateChanged(int);
    void startAnimation();
    void stopAnimation();
};

#endif // NOWPLAYINGINDICATOR_H
