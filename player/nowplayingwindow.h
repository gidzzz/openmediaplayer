#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include "basewindow.h"

#include <QtGui>
#include <QtDBus>
#include <QTimer>

#include "mirror.h"
#include "cqgraphicsview.h"
#include "ui_nowplayingwindow.h"
#include "includes.h"
#include "confirmdialog.h"
#include "ringtonedialog.h"
#include "sharedialog.h"
#include "delegates/playlistdelegate.h"
#include "qmlview.h"
#include "texteditautoresizer.h"
#include "coverpicker.h"
#include "editlyrics.h"
#include "mediaart.h"
#include "playlistquerymanager.h"
#include "playlistpicker.h"
#include "lyricsmanager.h"
#include "rotator.h"

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
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

class NowPlayingWindow : public BaseWindow
{
    Q_OBJECT

public:
    static NowPlayingWindow* acquire(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    static void destroy();
    ~NowPlayingWindow();
    QString currentURI;
    QString currentMIME;
    QString defaultWindowTitle;
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void hidden();
    void itemDropped(QListWidgetItem *item, int from);

public slots:
    void setLyrics(QString lyrics);
    void reloadLyrics();
    void setAlbumImage(QString);
    void onShuffleButtonToggled(bool);
#ifdef MAFW
    void updatePlaylistState();
#endif
    void showCarView();
    void showEntertainmentView();

private:
    static NowPlayingWindow *instance;
    explicit NowPlayingWindow(QWidget *parent, MafwAdapterFactory *mafwFactory);
    Ui::NowPlayingWindow *ui;
    QmlView *qmlView;
    int playlistTime;
    int currentViewId;
    int swipeStart;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwSourceAdapter *metadataSource;
    MafwPlaylistAdapter* playlist;
    PlaylistQueryManager *playlistQM;
    LyricsManager *lyricsManager;
    int mafwState;
    GConfItem *lastPlayingSong;
    QString metadataObjectId;
    bool event(QEvent *event);
    void showEvent(QShowEvent *);
#endif
    void setButtonIcons();
    void setTitle(QString title);
    void setArtist(QString artist);
    void setAlbum(QString album);
    void setSongNumber(int currentSong, int numberOfSongs);
    void updatePlaylistTimeLabel();
    void startPositionTimer();
    void connectSignals();
    QTimer *keyTimer;
    QTimer *clickTimer;
    QTimer *volumeTimer;
    QTimer *positionTimer;
    bool playlistRequested;
    bool isMediaSeekable;
    bool buttonWasDown;
    bool enableLyrics;
    bool lazySliders;
    bool permanentDelete;
    bool reverseTime;
    bool dragInProgress;
    bool portrait;
    QListWidgetItem *clickedItem;
    int songDuration;
    int currentSongPosition;
    QGraphicsScene *albumArtSceneLarge;
    QGraphicsScene *albumArtSceneSmall;
    QString albumArtPath;
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void closeEvent(QCloseEvent *);

private slots:
    void selectAlbumArt();
    void resetAlbumArt();
    void refreshAlbumArt();
    void editLyrics();
    void reloadLyricsOverridingCache();
    void toggleVolumeSlider();
    void showFMTXDialog();
    void cycleView(int direction = 1);
    void cycleViewBack();
    void onKeyTimeout();
    void forgetClick();
    void onItemDoubleClicked();
    void onItemDropped(QListWidgetItem *item, int from);
#ifdef MAFW
    void onItemMoved(guint from, guint to);
    void onPropertyChanged(const QDBusMessage &msg);
    void onStateChanged(int state);
    void onPositionChanged(int, QString);
    void onGetStatus(MafwPlaylist*, uint index, MafwPlayState state, const char* ,QString);
    void onRendererMetadataRequested(GHashTable*, QString objectId, QString);
    void onSourceMetadataRequested(QString, GHashTable*, QString);
    void onGetPlaylistItems(QString object_id, GHashTable *metadata, guint index);
    void setPosition(int newPosition);
    void onPlaylistItemActivated(QListWidgetItem*);
    void clearPlaylist();
    void onGconfValueChanged();
    void onMediaChanged(int index, char*);
    void onMediaIsSeekable(bool seekable);
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void updatePlaylist(guint from = -1, guint nremove = 0, guint nreplace = 0);
#endif
    void togglePlayback();
    void onMetadataChanged(QString name, QVariant value);
    void volumeWatcher();
    void onRepeatButtonToggled(bool checked);
    void orientationChanged(int w, int h);
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onPositionSliderPressed();
    void onPositionSliderReleased();
    void onPositionSliderMoved(int position);
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();

    void onPlayMenuRequested(const QPoint &pos);
    void onLyricsContextMenuRequested(const QPoint &pos);
    void onViewContextMenuRequested(const QPoint &pos);
    void onContextMenuRequested(const QPoint &pos);

    void onDeleteClicked();
    void onRingtoneClicked();
    void onShareClicked();
    void createQmlView(QUrl source, QString title);
    void updateQmlViewMetadata();
    void nullQmlView();
    void onAddAllToPlaylist();
    void onDeleteFromNowPlaying();
    void onAddToPlaylist();
    void selectItemByRow(int row);
    void focusItemByRow(int row);
#ifdef Q_WS_MAEMO_5
    void onScreenLocked(bool locked);
#endif
};

#endif // NOWPLAYINGWINDOW_H
