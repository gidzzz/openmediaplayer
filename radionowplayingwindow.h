#ifndef RADIONOWPLAYINGWINDOW_H
#define RADIONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QGraphicsView>

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
#endif

#include "ui_radionowplayingwindow.h"
#include "includes.h"
#include "rotator.h"
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

class RadioNowPlayingWindow : public QMainWindow
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

    void setAlbumImage(QString image);
    QGraphicsScene *albumArtScene;

    QTimer *volumeTimer;
    QTimer *positionTimer;
    bool lazySliders;
    bool buttonWasDown;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwSource;
    MafwPlaylistAdapter* playlist;
    int mafwState;
    QString currentObjectId;
    QString artist;
    QString title;
    QString uri;
#endif

private slots:
    void togglePlayback();
    void toggleVolumeSlider();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void showWindowMenu();
    void orientationChanged(int w, int h);
    void volumeWatcher();
#ifdef Q_WS_MAEMO_5
    void showFMTXDialog();
#endif
    void showBookmarkDialog();
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onStopButtonPressed();
    void streamIsSeekable(bool seekable);
    void updateSongLabel();
#ifdef MAFW
    void onStateChanged(int state);
    void onMediaChanged(int, char* objectId);
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
    void onRendererMetadataRequested(GHashTable*, QString, QString error);
    void onSourceMetadataRequested(QString objectId, GHashTable *metadata, QString error);
    void onRendererMetadataChanged(QString name, QVariant value);
#endif
};

#endif // RADIONOWPLAYINGWINDOW_H
