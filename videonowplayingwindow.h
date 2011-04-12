#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include <QDesktopWidget>
#include <QKeyEvent>

#include "ui_videonowplayingwindow.h"
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include <QSpacerItem>
#endif

#ifdef MAFW
    #include "mafwadapterfactory.h"
#else
    class MafwRendererAdapter;
    class MafwSourceAdapter;
    class MafwPlaylistAdapter;
#endif


namespace Ui {
    class VideoNowPlayingWindow;
}

class VideoNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoNowPlayingWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~VideoNowPlayingWindow();
    void playObject(QString objectId);

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    void showOverlay(bool show);
    QTimer *volumeTimer;
    QTimer *positionTimer;
    QString objectIdToPlay;
    bool portrait;
    bool isOverlayVisible;
    bool gotInitialState;
#ifdef Q_WS_MAEMO_5
    void setDNDAtom(bool dnd);
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    int colorkey;
    int mafwState;
    int length;
    int pausedPosition;
    bool errorOccured;
#endif

private slots:
    void toggleVolumeSlider();
    void volumeWatcher();
    void orientationChanged();
#ifdef MAFW
    void onPropertyChanged(const QDBusMessage &msg);
    void stateChanged(int state);
    void onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString);
    void onPositionChanged(int position, QString);
    void onSourceMetadataRequested(QString, GHashTable *metadata, QString error);
    void playVideo();
    void onErrorOccured(const QDBusMessage &msg);
#endif
#ifdef Q_WS_MAEMO_5
    void onPortraitMode();
    void onLandscapeMode();
#endif
    void onSliderMoved(int);
};

#endif // VIDEONOWPLAYINGWINDOW_H
