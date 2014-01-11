#ifndef NOWPLAYINGINDICATOR_H
#define NOWPLAYINGINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QVector>
#include <QTimer>
#include <QMouseEvent>

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#ifdef Q_WS_MAEMO_5
    #include "maemo5deviceevents.h"
#endif

#include "ui_nowplayingindicator.h"
#include "includes.h"
#include "nowplayingwindow.h"
#include "radionowplayingwindow.h"
#include "videonowplayingwindow.h"
#include "kbmenu.h"

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

#ifdef MAFW
    void setFactory(MafwAdapterFactory *mafwFactory = 0);
#endif

public slots:
    void togglePlayback();
    void autoSetVisibility();
    void inhibit();
    void restore();
    void poke();

private:
    Ui::NowPlayingIndicator *ui;

    void paintEvent(QPaintEvent*);
    void contextMenuEvent(QContextMenuEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

    void connectSignals();
    void stopAnimation();
    void triggerAnimation();

    QWidget *parentToReenable;
    QMainWindow *window;
    QVector<QPixmap> images;
    QTimer *animationTimer;
    QTimer *pokeTimer;
    bool ready;
    bool poked;
    int inhibited;
    int frame;

#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter *mafwrenderer;
    MafwPlaylistAdapter *playlist;
    int mafwState;
#endif
#ifdef Q_WS_MAEMO_5
    Maemo5DeviceEvents *deviceEvents;
#endif

private slots:
#ifdef Q_WS_MAEMO_5
    void onTkLockChanged(bool);
#endif
#ifdef MAFW
    void onStateChanged(int);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString);
    void onPlaylistReady(MafwPlaylist*,uint,MafwPlayState, const char* objectId, QString);
#endif
    void nextFrame();
    void openWindow();
    void onWindowDestroyed();
    void onNowPlayingWindowHidden();
    void onPokeTimeout();
    void onAudioPlaylistSelected();
};

#endif // NOWPLAYINGINDICATOR_H
