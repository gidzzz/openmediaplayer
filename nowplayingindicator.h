#ifndef NOWPLAYINGINDICATOR_H
#define NOWPLAYINGINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QTimer>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <mafwrendereradapter.h>
#include "ui_nowplayingindicator.h"

#define idleFrame "/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator_pause.png"

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
    void setBackgroundImage(const QString &image);
    void mouseReleaseEvent(QMouseEvent *);
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
