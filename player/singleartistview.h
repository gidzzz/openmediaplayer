#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include "basewindow.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "ui_singleartistview.h"
#include "includes.h"
#include "confirmdialog.h"
#include "singlealbumview.h"
#include "delegates/thumbnailitemdelegate.h"
#include "headerawareproxymodel.h"
#include "currentplaylistmanager.h"

namespace Ui {
    class SingleArtistView;
}

class SingleArtistView : public BaseWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleArtistView();
    bool eventFilter(QObject *, QEvent *e);
    void browseArtist(QString objectId);

private:
    Ui::SingleArtistView *ui;

    QStandardItemModel *albumModel;
    QSortFilterProxyModel *albumProxyModel;

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseArtistId;
    uint playlistToken;
    QString artistObjectId;
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
    void onArtistAddFinished(uint token, int count);
    void onAlbumAddFinished(uint token, int count);
    void browseAllAlbums(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error);
    void onContainerChanged(QString objectId);
#endif
    void onAlbumSelected(QModelIndex index);
    void orientationChanged(int w, int h);
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString text);
    void addAllToNowPlaying();
    void deleteCurrentArtist();
    void shuffleAllSongs();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onAddAlbumToNowPlaying();
    void onNowPlayingWindowHidden();
    void onChildClosed();
    void onDeleteClicked();
};

#endif // SINGLEARTISTVIEW_H
