#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtDBus>
#include <QTimer>

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "mirror.h"
#include "cqgraphicsview.h"
#include "ui_nowplayingwindow.h"
#include "includes.h"
#include "delegates/playlistdelegate.h"
#include "entertainmentview.h"
#include "carview.h"

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
    #include "share.h"
    #include "maemo5deviceevents.h"
#endif

#ifdef MAFW
    #include <mafw/mafwadapterfactory.h>
    #include <gq/GConfItem>
    #include "mafw/mafwplaylistmanageradapter.h"
#else
    class MafwRendererAdapter;
    class MafwSourceAdapter;
    class MafwPlaylistAdapter;
#endif
namespace Ui {
    class NowPlayingWindow;
}

class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    static NowPlayingWindow* acquire(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~NowPlayingWindow();
    QString albuminfolder;
    QNetworkAccessManager* data;
    QString TEartist, TEalbum, TEtitle, TEid;

signals:
    void hidden();

public slots:
    void reloadLyricsFromFile();
    void onSongSelected(int, int, QString, QString, QString, int);
    void setAlbumImage(QString);
    void onShuffleButtonToggled(bool);
    void closeEvent(QCloseEvent *e);
#ifdef MAFW
    void updatePlaylistState();
#endif

private:
    QTime t; int n;
    static NowPlayingWindow *instance;
    explicit NowPlayingWindow(QWidget *parent, MafwAdapterFactory *mafwFactory);
    Ui::NowPlayingWindow *ui;
    EntertainmentView *entertainmentView;
    CarView *carView;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    MafwPlaylist *mafwPlaylist;
    MafwPlaylistManagerAdapter *mafw_playlist_manager;
    int mafwState;
    GConfItem *lastPlayingSong;
    void showEvent(QShowEvent *);
    gpointer browseId;
#endif
    void setButtonIcons();
    void setSongNumber(int currentSong, int numberOfSongs);
    void connectSignals();
    QTimer *volumeTimer;
    QTimer *positionTimer;
    QLineEdit *playlistNameLineEdit;
    bool playlistRequested;
    bool isDefaultArt;
    bool buttonWasDown;
    int songDuration;
    int currentSongPosition;
    int numberOfSongsToAdd;
    QGraphicsScene *albumArtScene;
    QString albumArtUri;
    mirror *m;
    void keyPressEvent(QKeyEvent *);
    QMenu *contextMenu;
    QDialog *savePlaylistDialog;
#ifdef Q_WS_MAEMO_5
    Maemo5DeviceEvents *deviceEvents;
#endif

private slots:
    QString cleanItem(QString data = "");
    void on_lyricsText_customContextMenuRequested(QPoint pos);
    void onLyricsDownloaded(QNetworkReply *reply);
    void on_view_customContextMenuRequested(QPoint pos);
    void changeArt();
    void editLyrics();
    void reloadLyrics();
    void toggleVolumeSlider();
    void showFMTXDialog();
    void toggleList();
    void setRingingTone();
#ifdef MAFW
    void onPropertyChanged(const QDBusMessage &msg);
    void stateChanged(int state);
    void onPositionChanged(int, QString);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString);
    void onRendererMetadataRequested(GHashTable*, QString object_id, QString);
    void onSourceMetadataRequested(QString, GHashTable*, QString);
    void onGetPlaylistItems(QString object_id, GHashTable *metadata, guint index);
    void setPosition(int);
    void onPlaylistItemActivated(QListWidgetItem*);
    void clearPlaylist();
    void onPlaylistChanged();
    void onGconfValueChanged();
    void onMediaChanged(int index, char*);
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void updatePlaylist();
    void onRingingToneUriReceived(QString objectId, QString uri);
    void onShareUriReceived(QString objectId, QString uri);
#endif
    void editTags();
    void metadataChanged(QString name, QVariant value);
    void volumeWatcher();
    void onRepeatButtonToggled(bool);
    void orientationChanged();
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onPositionSliderMoved(int position);
    void onContextMenuRequested(const QPoint &point);
    void onShareClicked();
    void showEntertainmentView();
    void updateEntertainmentViewMetadata();
    void nullEntertainmentView();
    void showCarView();
    void updateCarViewMetadata();
    void nullCarView();
    void savePlaylist();
    void onSavePlaylistAccepted();
    void onDeleteFromNowPlaying();
    void selectItemByText(int numberInPlaylist);
#ifdef Q_WS_MAEMO_5
    void onScreenLocked(bool locked);
#endif
};

#endif // NOWPLAYINGWINDOW_H
