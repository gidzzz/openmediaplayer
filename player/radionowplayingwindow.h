#ifndef RADIONOWPLAYINGWINDOW_H
#define RADIONOWPLAYINGWINDOW_H

#include "basewindow.h"

#include <QTimer>
#include <QTime>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QGraphicsView>

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
    #include "maemo5deviceevents.h"
#endif

#include "ui_radionowplayingwindow.h"
#include "includes.h"
#include "rotator.h"
#include "missioncontrol.h"
#include "bookmarkdialog.h"
#include "mirror.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#else
    class MafwRendererAdapter;
    class MafwSourceAdapter;
    class MafwPlaylistAdapter;
#endif

namespace Ui {
    class RadioNowPlayingWindow;
}

class RadioNowPlayingWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit RadioNowPlayingWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~RadioNowPlayingWindow();

public slots:
    void play();

private:
    Ui::RadioNowPlayingWindow *ui;

    void keyPressEvent(QKeyEvent *e);

    void connectSignals();
    void setIcons();

    void startPositionTimer();

    void updateSongLabel();

    void setAlbumImage(QString image);
    QGraphicsScene *albumArtScene;

    QNetworkSession *networkSession;

    QTimer *volumeTimer;
    QTimer *positionTimer;
    bool lazySliders;
    bool buttonWasDown;
    bool isMediaSeekable;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwPlaylistAdapter* playlist;
    int mafwState;
    QString artist;
    QString title;
    QString uri;
#endif

private slots:
    void togglePlayback();
    void toggleVolumeSlider();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void onOrientationChanged(int w, int h);
    void onScreenLocked(bool locked);
    void volumeWatcher();
#ifdef Q_WS_MAEMO_5
    void showFMTXDialog();
#endif
    void showBookmarkDialog();
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onStopButtonPressed();
    void onMetadataChanged(QString key, QVariant value);
#ifdef MAFW
    void onStateChanged(int state);
    void onMediaChanged(int, char *);
    void onPropertyChanged(const QDBusMessage &msg);
    void onGetStatus(MafwPlaylist*, uint index, MafwPlayState state, const char *, QString);
    void onGetPosition(int position, QString);
    void onBufferingInfo(float status);
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void onPlayMenuRequested(const QPoint &pos);
    void onPositionSliderPressed();
    void onPositionSliderReleased();
    void onPositionSliderMoved(int position);
#endif
};

#endif // RADIONOWPLAYINGWINDOW_H
