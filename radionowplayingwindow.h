#ifndef RADIONOWPLAYINGWINDOW_H
#define RADIONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDesktopWidget>
#include <QTime>
#include <QNetworkSession>

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#else
    class MafwRendererAdapter;
    class MafwSourceAdapter;
    class MafwPlaylistAdapter;
#endif
#include "includes.h"

namespace Ui {
    class RadioNowPlayingWindow;
}

class RadioNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RadioNowPlayingWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~RadioNowPlayingWindow();

private:
    Ui::RadioNowPlayingWindow *ui;
    void connectSignals();
    void setIcons();
    QTimer *volumeTimer;
    QTimer *positionTimer;
    bool buttonWasDown;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwRadioSource;
    MafwPlaylistAdapter* playlist;
    int mafwState;
    int streamDuration;
    QString artistName;
    QString albumName;
#endif

private slots:
    void toggleVolumeSlider();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void orientationChanged();
    void volumeWatcher();
#ifdef Q_WS_MAEMO_5
    void showFMTXDialog();
#endif
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onStopButtonPressed();
    void streamIsSeekable(bool seekable);
    void updateArtistAlbum();
#ifdef MAFW
    void onStateChanged(int state);
    void onMediaChanged(int, char* objectId);
    void onPropertyChanged(const QDBusMessage &msg);
    void onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString);
    void onGetPosition(int position, QString);
    void onBufferingInfo(float);
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void onRendererMetadataRequested(GHashTable*, QString object_id, QString error);
    void onSourceMetadataRequested(QString, GHashTable *metadata, QString error);
    void onRendererMetadataChanged(QString name, QVariant value);
#endif
};

#endif // RADIONOWPLAYINGWINDOW_H
