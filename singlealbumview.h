#ifndef SINGLEALBUMVIEW_H
#define SINGLEALBUMVIEW_H

#include <QMainWindow>
#include <QTime>

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5ValueButton>
#else
    #include <QPushButton>
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "ui_singlealbumview.h"
#include "includes.h"
#include "nowplayingwindow.h"
#include "delegates/singlealbumviewdelegate.h"


namespace Ui {
    class SingleAlbumView;
}

class SingleAlbumView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleAlbumView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleAlbumView();
#ifdef MAFW
    void browseAlbumByObjectId(QString objectId);
    bool isSingleAlbum;
#endif

private:
    Ui::SingleAlbumView *ui;
    QString albumObjectId;
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleButton;
    void notifyOnAddedToNowPlaying(int songCount);
#else
    QPushButton *shuffleButton;
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseAllSongsId;
#endif
    void setupShuffleButton();
    void updateSongCount();

private slots:
    void orientationChanged();
#ifdef MAFW
    void listSongs();
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onItemActivated(QListWidgetItem *item);
    void onRingingToneUriReceived(QString objectId, QString uri);
    void onShareUriReceived(QString objectId, QString uri);
    void onContainerChanged(QString objectId);
#endif
    void createPlaylist(bool);
    void onShuffleButtonClicked();
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString);
    void addAllToNowPlaying();
    void deleteCurrentAlbum();
    void onContextMenuRequested(const QPoint &point);
    void setRingingTone();
    void onShareClicked();
    void onDeleteClicked();
    void onAddToNowPlaying();
    void onNowPlayingWindowHidden();
};

#endif // SINGLEALBUMVIEW_H
