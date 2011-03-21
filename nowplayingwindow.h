#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtDBus>
#include <QTimer>

#include "mirror.h"
#include "cqgraphicsview.h"
#include "ui_nowplayingwindow.h"
#include "includes.h"
#include "delegates/playlistdelegate.h"
#include "entertainmentview.h"

#ifdef Q_WS_MAEMO_5
    #include "fmtxdialog.h"
    #include "share.h"
#endif

#ifdef MAFW
    #include "mafwrendereradapter.h"
    #include "mafwplaylistadapter.h"
    #include "mafw-1.0/libmafw-shared/mafw-playlist-manager.h"
    #include "mafwsourceadapter.h"
    #include <gq/GConfItem>
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
    explicit NowPlayingWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0, MafwPlaylistAdapter* pls = 0);
    ~NowPlayingWindow();
#ifdef MAFW
    void updatePlaylistState();
#endif

public slots:
    void onSongSelected(int, int, QString, QString, QString, int);
    void setAlbumImage(QString);
    void onShuffleButtonToggled(bool);

private:
    Ui::NowPlayingWindow *ui;
    EntertainmentView *entertainmentView;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
    MafwPlaylistAdapter *playlist;
    MafwPlaylist *mafwPlaylist;
    int mafwState;
    GConfItem *lastPlayingSong;
    void showEvent(QShowEvent *);
#endif
    void setButtonIcons();
    void setSongNumber(int currentSong, int numberOfSongs);
    void connectSignals();
    QTimer *volumeTimer;
    QTimer *positionTimer;
    bool playlistRequested;
    bool isDefaultArt;
    int songDuration;
    QGraphicsScene *albumArtScene;
    QString albumArtUri;
    mirror *m;
    void keyPressEvent(QKeyEvent *);
    QMenu *contextMenu;

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
    void toggleList();
    void setRingingTone();
#ifdef MAFW
    void onVolumeChanged(const QDBusMessage &msg);
    void stateChanged(int state);
    void onPositionChanged(int, QString);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString);
    void onRendererMetadataRequested(QString, QString, QString, QString object_id, QString);
    void onSourceMetadataRequested(QString, GHashTable*, QString);
    void onGetPlaylistItems(QString object_id, GHashTable *metadata, guint index);
    void setPosition(int);
    void onPlaylistItemActivated(QListWidgetItem*);
    void clearPlaylist();
    void onPlaylistChanged();
    void onGconfValueChanged();
    void onMediaChanged(int index, char*);
#endif
    void metadataChanged(QString name, QVariant value);
    void volumeWatcher();
    void onRepeatButtonToggled(bool);
    void orientationChanged();
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onPositionSliderMoved(int position);
    void onContextMenuRequested(const QPoint &point);
    void onShareClicked();
    void showEntertainmentview();
    void updateEntertainmentViewMetadata();
    void nullEntertainmentView();
    void savePlaylist();
};

#endif // NOWPLAYINGWINDOW_H
