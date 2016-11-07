#ifndef SINGLEALBUMVIEW_H
#define SINGLEALBUMVIEW_H

#include "browserwindow.h"

#include "mafw/mafwregistryadapter.h"

#include "includes.h"
#include "confirmdialog.h"
#include "metadatadialog.h"
#include "nowplayingwindow.h"
#include "delegates/singlealbumviewdelegate.h"
#include "delegates/shufflebuttondelegate.h"

class SingleAlbumView : public BrowserWindow
{
    Q_OBJECT

public:
    explicit SingleAlbumView(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);
    void browseAlbumByObjectId(QString objectId);

private:
    void notifyOnAddedToNowPlaying(int songCount);
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwTrackerSource;
    CurrentPlaylistAdapter *playlist;
    QString albumObjectId;
    uint browseAlbumId;

private slots:
    void listSongs();
    void updateSongCount();
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata);
    void onItemActivated(QModelIndex index);
    void onContainerChanged(QString objectId);
    int appendAllToPlaylist(bool filter);
    void addAllToNowPlaying();
    void deleteCurrentAlbum();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onRingtoneClicked();
    void onShareClicked();
    void onDetailsClicked();
    void onDeleteClicked();
    void onAddToNowPlaying();
    void onAddToPlaylist();
    void onNowPlayingWindowHidden();
};

#endif // SINGLEALBUMVIEW_H
