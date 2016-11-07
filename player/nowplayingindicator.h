#ifndef NOWPLAYINGINDICATOR_H
#define NOWPLAYINGINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QVector>
#include <QTimer>
#include <QMouseEvent>

#include "mafw/mafwregistryadapter.h"

#include "maemo5deviceevents.h"

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

    void setRegistry(MafwRegistryAdapter *mafwRegistry = 0);

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

    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    CurrentPlaylistAdapter *playlist;
    MafwPlayState mafwState;
    Maemo5DeviceEvents *deviceEvents;

private slots:
    void onTkLockChanged(bool);
    void onStateChanged(MafwPlayState state);
    void onStatusReceived(MafwPlaylist *, uint, MafwPlayState state);
    void onPlaylistReady();
    void nextFrame();
    void openWindow();
    void onWindowDestroyed();
    void onNowPlayingWindowHidden();
    void onPokeTimeout();
    void onAudioPlaylistSelected();
};

#endif // NOWPLAYINGINDICATOR_H
