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
    #include "mafwsourceadapter.h"
    #include "mafwplaylistadapter.h"
    #include "maemo5deviceevents.h"
#endif
#include "ui_nowplayingindicator.h"
#include "includes.h"
#include "nowplayingwindow.h"
#include "radionowplayingwindow.h"
#include "videonowplayingwindow.h"

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
    void triggerAnimation();
#ifdef MAFW
    void setSources(MafwRendererAdapter *renderer, MafwSourceAdapter *source, MafwPlaylistAdapter *pls);
#endif

public slots:
    void stopAnimation();

signals:
    void clicked();

private:
    Ui::NowPlayingIndicator *ui;
    void paintEvent(QPaintEvent*);
    void mouseReleaseEvent(QMouseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void connectSignals();
    QList<QPixmap> images;
    QTimer *timer;
    int mafwState;
    QString indicatorImage;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
#endif
#ifdef Q_WS_MAEMO_5
    Maemo5DeviceEvents *deviceEvents;
#endif
    int frame;

private slots:
#ifdef Q_WS_MAEMO_5
    void onStateChanged(int);
    void onTkLockChanged(bool);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString);
#endif
    void startAnimation();
};

#endif // NOWPLAYINGINDICATOR_H
