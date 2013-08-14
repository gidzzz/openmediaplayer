#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "basewindow.h"

#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QtGui>
#include <QDBusConnection>

#include "musicwindow.h"
#include "videoswindow.h"
#include "internetradiowindow.h"
#include "ui_mainwindow.h"
#include "nowplayingindicator.h"
#include "includes.h"
#include "settingsdialog.h"
#include "sleeperdialog.h"
#include "aboutwindow.h"
#include "delegates/maindelegate.h"
#include "rotator.h"
#include "upnpcontrol.h"
#include "opendialog.h"
#include "currentplaylistmanager.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include <QtGui/QX11Info>
    #include <X11/Xlib.h>
    #define DBUS_SERVICE   "com.nokia.mediaplayer"
    #define DBUS_PATH      "/com/nokia/osso/mediaplayer"
    #define DBUS_INTERFACE "com.nokia.mediaplayer"
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
    #include <libgnomevfs/gnome-vfs-mime-utils.h>
    #define TAGSOURCE_AUDIO_PATH     "localtagfs::music/songs"
    #define TAGSOURCE_PLAYLISTS_PATH "localtagfs::music/playlists"
    #define TAGSOURCE_VIDEO_PATH     "localtagfs::videos"
    #define RADIOSOURCE_PATH         "iradiosource::"
#endif

namespace Ui {
    class MainWindow;
}

class MainWindow : public BaseWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.mediaplayer")

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    Q_SCRIPTABLE void open_mp_main_view();
    Q_SCRIPTABLE void open_mp_now_playing();
    Q_SCRIPTABLE void open_mp_now_playing_playback_on();
    Q_SCRIPTABLE void open_mp_radio_playing();
    Q_SCRIPTABLE void open_mp_radio_playing_playback_on();
    Q_SCRIPTABLE void open_mp_car_view();
    Q_SCRIPTABLE void mime_open(const QString &uri);
    Q_SCRIPTABLE void play_automatic_playlist(const QString &playlistName, bool shuffle = false);
    Q_SCRIPTABLE void play_saved_playlist(const QString &playlistName, bool shuffle = false);
    Q_SCRIPTABLE void top_application();

signals:
    void sleeperSet(qint64 timestamp);

private:
    Ui::MainWindow *ui;
    MusicWindow *musicWindow;
    UpnpControl *upnpControl;
    void paintEvent(QPaintEvent*);
    void closeEvent(QCloseEvent *);
    void loadThemeIcons();
    void setButtonIcons();
    void connectSignals();
    void closeChildren();
    QTimer *sleeperTimer;
    QTimer *sleeperVolumeTimer;
    QTimer *resumeTimer;
    qint64 sleeperStartStamp;
    qint64 sleeperTimeoutStamp;
    int volume;
    int volumeReduction;
    void scheduleSleeperVolume();
#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox *updatingInfoBox;
    QProgressBar *updatingProgressBar;
    QLabel *updatingLabel;
    bool updatingShow;
    bool pausedByCall;
    bool wasRinging;
    bool wiredHeadsetIsConnected;
    qint64 headsetPauseStamp;
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwSourceAdapter *mafwRadioSource;
    MafwPlaylistAdapter* playlist;
    uint browsePlaylistId;
    uint playlistToken;
    void countSongs();
    void countVideos();
    int mafwState;
    QString objectIdToPlay;
#endif

private slots:
    void orientationChanged(int w, int h);
    void showAbout();
    void processListClicks(QListWidgetItem*);
    void openSettings();
    void openSleeperDialog();
    void reloadSettings();
    void showMusicWindow();
    void showVideosWindow();
    void showInternetRadioWindow();
    void onShuffleAllClicked();
    NowPlayingWindow* createNowPlayingWindow();
    void createVideoNowPlayingWindow();
    void onChildOpened();
    void onNowPlayingWindowHidden();
    void onChildClosed();
    void setSleeperTimer(int seconds, int reduction);
    void stepSleeperVolume();
    void onSleeperTimeout();
#ifdef MAFW
    void onPropertyChanged(const QDBusMessage &msg);
    void setupPlayback();
    void getInitialVolume(int volume);
    void trackerSourceReady();
    void radioSourceReady();
    void countAudioVideoResult(QString objectId, GHashTable* metadata, QString error);
    void countRadioResult(QString objectId, GHashTable *metadata, QString error);
    void countRadioStations();
    void onAddFinished(uint token);
    void onSourceUpdating(int progress, int processed_items, int remaining_items, int remaining_time);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState state,const char*,QString);
    void togglePlayback();
    void onStateChanged(int state);
    void onContainerChanged(QString objectId);
    void openDirectory(QString uri, Media::Type type);
#endif
#ifdef Q_WS_MAEMO_5
    void registerDbusService();
    void onWirelessHeadsetConnected();
    void onHeadsetConnected();
    void onHeadsetDisconnected();
    void onHeadsetButtonPressed(QDBusMessage msg);
    void onCallStateChanged(QDBusMessage msg);
    void onScreenLocked(bool locked);
    void phoneButton();
    void updateWiredHeadset();
    void takeScreenshot();
#endif

};

#endif // MAINWINDOW_H
