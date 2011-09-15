#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include "aboutwindow.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #define DBUS_SERVICE   "com.nokia.mediaplayer"
    #define DBUS_PATH      "/com/nokia/osso/mediaplayer"
    #define DBUS_INTERFACE "com.nokia.mediaplayer"
#endif
#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
    #include <libgnomevfs/gnome-vfs-mime-utils.h>
#endif

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.mediaplayer")

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    Q_SCRIPTABLE void open_mp_now_playing();
    Q_SCRIPTABLE void mime_open(const QString &uri);

private:
    Ui::MainWindow *ui;
    MusicWindow *myMusicWindow;
    VideosWindow *myVideosWindow;
    InternetRadioWindow *myInternetRadioWindow;
    void paintEvent(QPaintEvent*);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);
    bool shuffleNowPlayingWindowCreated;

    void loadThemeIcons();
    void setButtonIcons();
    void connectSignals();
    void setLabelText();
    QMainWindow *dbusNowPlaying;
    QString uriToPlay;
#ifdef Q_WS_MAEMO_5
    QMaemo5InformationBox *updatingIndex;
    QProgressBar *updatingProgressBar;
    QLabel *updatingLabel;
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwSourceAdapter *mafwRadioSource;
    MafwPlaylistAdapter* playlist;
    uint browseAllSongsId;
    const char* TAGSOURCE_AUDIO_PATH;
    const char* TAGSOURCE_VIDEO_PATH;
    const char* RADIOSOURCE_PATH;
    void countSongs();
    void countVideos();
    void countRadioStations();
    int mafwState;
    int rendererStatusNotifications;
    int songAddBufferSize;
    gchar** songAddBuffer;
#endif

private slots:
    void orientationChanged();
    void showAbout();
    void processListClicks(QListWidgetItem*);
    void openSettings();
    void showVideosWindow();
    void showInternetRadioWindow();
    void onShuffleAllClicked();
    void createNowPlayingWindow();
    void onDbusNpWindowDestroyed();
#ifdef MAFW
    void trackerSourceReady();
    void radioSourceReady();
    void countAudioVideoResult(QString objectId, GHashTable* metadata, QString error);
    void countRadioResult(QString objectId, GHashTable* metadata, QString error);
    void browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable*, QString);
    void onSourceUpdating(int progress, int processed_items, int remaining_items, int remaining_time);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState state,const char*,QString);
    void pausePlay();
    void onStateChanged(int state);
#endif
#ifdef Q_WS_MAEMO_5
    void onBluetoothButtonPressed(QDBusMessage msg);
    void takeScreenshot();
#endif
};

#endif // MAINWINDOW_H
