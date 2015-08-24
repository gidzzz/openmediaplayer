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
#include "lyricseditdialog.h"
#include "lyricssearchdialog.h"
#include "mediaart.h"
#include "playlistquerymanager.h"
#include "playlistpicker.h"
#include "rotator.h"
#include "missioncontrol.h"

#include "fmtxdialog.h"
#include "maemo5deviceevents.h"

#include <mafw/mafwregistryadapter.h>
#include "mafw/mafwplaylistmanageradapter.h"

namespace Ui {
    class NowPlayingWindow;
}

class NowPlayingWindow : public BaseWindow
{
    Q_OBJECT

public:
    static NowPlayingWindow* acquire(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistryAdapter = 0);
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
    void setLyricsNormal(QString lyrics);
    void setLyricsInfo(QString info);
    void onShuffleButtonToggled(bool);
    void updatePlaylistState();
    void showCarView();
    void showEntertainmentView();

private:
    static NowPlayingWindow *instance;
    explicit NowPlayingWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistryAdapter);
    Ui::NowPlayingWindow *ui;
    QmlView *qmlView;
    int playlistTime;
    int currentViewId;
    int swipeStart;
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwTrackerSource;
    CurrentPlaylistAdapter *playlist;
    PlaylistQueryManager *playlistQM;
    MafwPlayState mafwState;
    QString metadataObjectId;
    bool event(QEvent *event);
    void showEvent(QShowEvent *);
    void setButtonIcons();
    void setSong(int index);
    void setTitle(QString title);
    void setArtist(QString artist);
    void setAlbum(QString album);
    void setLyrics(QString text, QColor color);
    void setAlbumImage(QString image);
    void detectAlbumImage(QString image = QString());
    void updateSongNumber();
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
    int currentSong;
    int currentSongPosition;
    int songDuration;
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
    void searchLyrics();
    void toggleVolumeSlider();
    void showFMTXDialog();
    void cycleView(int direction = 1);
    void cycleViewBack();
    void onKeyTimeout();
    void forgetClick();
    void onItemDoubleClicked();
    void onItemDropped(QListWidgetItem *item, int from);
    void onItemMoved(uint from, uint to);
    void onPropertyChanged(const QDBusMessage &msg);
    void onStateChanged(MafwPlayState state);
    void onPositionChanged(int, QString);
    void onStatusReceived(MafwPlaylist *, uint index, MafwPlayState state, QString, QString);
    void onItemReceived(QString objectId, GHashTable *metadata, uint index);
    void setPosition(int newPosition);
    void onPlaylistItemActivated(QListWidgetItem*);
    void clearPlaylist();
    void onMediaChanged(int index, QString);
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void updatePlaylist(uint from = -1, uint nremove = 0, uint nreplace = 0);
    void togglePlayback();
    void volumeWatcher();
    void onMetadataChanged(QString key, QVariant value);
    void onRepeatButtonToggled(bool checked);
    void onOrientationChanged(int w, int h);
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
    void onScreenLocked(bool locked);
};

#endif // NOWPLAYINGWINDOW_H
