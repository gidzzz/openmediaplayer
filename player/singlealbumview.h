#ifndef SINGLEALBUMVIEW_H
#define SINGLEALBUMVIEW_H

#include "browserwindow.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "includes.h"
#include "confirmdialog.h"
#include "nowplayingwindow.h"
#include "delegates/singlealbumviewdelegate.h"
#include "delegates/shufflebuttondelegate.h"

class SingleAlbumView : public BrowserWindow
{
    Q_OBJECT

public:
    explicit SingleAlbumView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
#ifdef MAFW
    void browseAlbumByObjectId(QString objectId);
#endif

private:
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    QString albumObjectId;
    uint browseAlbumId;
#endif

private slots:
#ifdef MAFW
    void listSongs();
    void updateSongCount();
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onItemActivated(QModelIndex index);
    void onRingingToneUriReceived(QString objectId, QString uri);
    void onContainerChanged(QString objectId);
#endif
    int appendAllToPlaylist(bool filter);
    void addAllToNowPlaying();
    void deleteCurrentAlbum();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void setRingingTone();
    void onShareClicked();
    void onDeleteClicked();
    void onAddToNowPlaying();
    void onAddToPlaylist();
    void onNowPlayingWindowHidden();
};

#endif // SINGLEALBUMVIEW_H
