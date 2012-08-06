#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include <QMainWindow>
#include <QListWidgetItem>

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "ui_singleartistview.h"
#include "includes.h"
#include "confirmdialog.h"
#include "singlealbumview.h"
#include "delegates/thumbnailitemdelegate.h"

namespace Ui {
    class SingleArtistView;
}

class SingleArtistView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleArtistView();
    bool eventFilter(QObject *, QEvent *e);
    void browseAlbum(QString artistId);

private:
    Ui::SingleArtistView *ui;
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseAllAlbumsId;
    uint addToNowPlayingId;
    QString artistObjectId;
    gchar** songAddBuffer;
    int songAddBufferSize;
    int visibleSongs;
    bool shuffleRequested;
    void listAlbums();
#endif
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif
    void updateSongCount();

private slots:
#ifdef MAFW
    void browseAllAlbums(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error);
    void onBrowseAllSongs(uint, int remainingCount, uint, QString objectId, GHashTable*, QString);
    void onAddAlbumBrowseResult(uint, int remainingCount, uint, QString objectId, GHashTable*, QString);
    void onContainerChanged(QString objectId);
#endif
    void onAlbumSelected(QListWidgetItem*);
    void orientationChanged(int w, int h);
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString);
    void addAllToNowPlaying();
    void deleteCurrentArtist();
    void shuffleAllSongs();
    void onContextMenuRequested(const QPoint &pos);
    void onAddAlbumToNowPlaying();
    void onNowPlayingWindowHidden();
    void onChildClosed();
    void onDeleteClicked();
};

#endif // SINGLEARTISTVIEW_H
