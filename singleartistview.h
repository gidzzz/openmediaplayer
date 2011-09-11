#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include <QMainWindow>
#include <QListWidgetItem>

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "singlealbumview.h"
#include "delegates/thumbnailitemdelegate.h"
#include "includes.h"

namespace Ui {
    class SingleArtistView;
}

class SingleArtistView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleArtistView();
    void browseAlbum(QString artistId);
    void setSongCount(int songCount);

private:
    Ui::SingleArtistView *ui;
    void keyReleaseEvent(QKeyEvent *);
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseAllAlbumsId;
    uint addToNowPlayingId;
    QString artistObjectId;
    void listAlbums();
    int numberOfSongs;
    gchar** songAddBuffer;
    int songAddBufferSize;
    bool shuffleRequested;
#endif
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif

private slots:
#ifdef MAFW
    void browseAllAlbums(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error);
    void onBrowseAllSongs(uint, int remainingCount, uint, QString objectId, GHashTable*, QString);
    void onAddAlbumBrowseResult(uint, int remainingCount, uint, QString objectId, GHashTable*, QString);
#endif
    void onAlbumSelected(QListWidgetItem*);
    void orientationChanged();
    void onSearchTextChanged(QString);
    void addAllToNowPlaying();
    void shuffleAllSongs();
    void onContextMenuRequested(const QPoint &point);
    void onAddAlbumToNowPlaying();
};

#endif // SINGLEARTISTVIEW_H
