#ifndef NOWPLAYINGINDICATOR_H
#define NOWPLAYINGINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QTimer>
#include <QtDBus>
#include <QDesktopWidget>
#include <QMouseEvent>
#ifdef Q_WS_MAEMO_5
    #include "mafwrendereradapter.h"
    #include "maemo5deviceevents.h"
#endif
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
    void connectSignals();
    QList<QPixmap> images;
    QTimer *timer;
    int mafwState;
    QString indicatorImage;
#ifdef Q_WS_MAEMO_5
    MafwRendererAdapter* mafwrenderer;
    Maemo5DeviceEvents *deviceEvents;
#endif
    int frame;

private slots:
#ifdef Q_WS_MAEMO_5
    void onStateChanged(int);
    void onTkLockChanged(bool);
#endif
    void startAnimation();
    void stopAnimation();
};

#endif // NOWPLAYINGINDICATOR_H
