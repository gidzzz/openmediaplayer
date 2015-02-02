#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include "browserwindow.h"

#include "mafw/mafwregistryadapter.h"

#include "includes.h"
#include "confirmdialog.h"
#include "singlealbumview.h"
#include "delegates/thumbnailitemdelegate.h"
#include "currentplaylistmanager.h"

class SingleArtistView : public BrowserWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);
    void browseArtist(QString objectId);

private:
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseArtistId;
    uint playlistToken;
    QString artistObjectId;
    int visibleSongs;
    bool shuffleRequested;
    void listAlbums();
    void notifyOnAddedToNowPlaying(int songCount);
    void updateSongCount();

private slots:
    void onArtistAddFinished(uint token, int count);
    void onAlbumAddFinished(uint token, int count);
    void browseAllAlbums(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error);
    void onContainerChanged(QString objectId);
    void onAlbumSelected(QModelIndex index);
    void addAllToNowPlaying();
    void deleteCurrentArtist();
    void shuffleAllSongs();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onAddAlbumToNowPlaying();
    void onNowPlayingWindowHidden();
    void onDeleteClicked();
};

#endif // SINGLEARTISTVIEW_H
