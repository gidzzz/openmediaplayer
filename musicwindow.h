#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QStringList>
#include <QDirIterator>
#include <QMenu>
#include <QtGui>
#ifdef Q_WS_MAEMO_5
    #include <QMaemo5ValueButton>
#endif

#include "nowplayingwindow.h"
#include "share.h"
#include "ui_musicwindow.h"
#include "delegates/songlistitemdelegate.h"
#include "delegates/artistlistitemdelegate.h"
#include "delegates/thumbnailitemdelegate.h"
#include "singlealbumview.h"
#include "singleartistview.h"
#include "singleplaylistview.h"
#include "singlegenreview.h"
#include "ui_musicwindow.h"
#include "includes.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
    #include "mafw/mafwplaylistmanageradapter.h"
#else
    class MafwRendererAdapter;
#endif

namespace Ui {
    class MusicWindow;
}

class MusicWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MusicWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~MusicWindow();
    bool eventFilter(QObject *obj, QEvent *e);

protected:
    void hideEvent(QHideEvent *);
    void showEvent(QShowEvent *);

signals:
    void hidden();
    void shown();

private:
    Ui::MusicWindow *ui;

    QStandardItemModel *songModel;
    QStandardItemModel *albumModel;
    QStandardItemModel *artistModel;
    QStandardItemModel *genresModel;
    QStandardItemModel *playlistModel;
    QSortFilterProxyModel *songProxyModel;
    QSortFilterProxyModel *albumProxyModel;
    QSortFilterProxyModel *artistProxyModel;
    QSortFilterProxyModel *genresProxyModel;
    QSortFilterProxyModel *playlistProxyModel;

    QDialog *renamePlaylistDialog;
    QLineEdit *playlistNameEdit;

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    MafwPlaylistManagerAdapter* mafwPlaylistManager;
    unsigned int browseAllSongsId;
    unsigned int browseAllArtistsId;
    unsigned int browseAllAlbumsId;
    unsigned int browseAllGenresId;
    unsigned int browseAllPlaylistsId;
    // Automatic playlists
    unsigned int browseRecentlyAddedId;
    unsigned int browseRecentlyPlayedId;
    unsigned int browseMostPlayedId;
    unsigned int browseNeverPlayedId;
    // Imported playlists
    unsigned int browseImportedPlaylistsId;
    int recentlyAddedCount;
    int recentlyPlayedCount;
    int mostPlayedCount;
    int neverPlayedCount;
    void fetchUri(QString objectId);
    uint addToNowPlayingId;
    int numberOfSongsToAdd;
    gchar** songAddBuffer;
    int songAddBufferSize;
#endif
    void connectSignals();
    void disconnectSearch();
    void populateMenuBar();
    void hideLayoutContents();
    void saveViewState(QVariant);
    void loadViewState();
    QListView* currentList();
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif

private slots:
    void onContextMenuRequested(QPoint);
    void onSongSelected(QModelIndex index);
    void setRingingTone();
    void onShareClicked();
    void onDeleteClicked();
    void orientationChanged(int w, int h);
    void showAlbumView();
    void showPlayListView();
    void showArtistView();
    void showSongsView();
    void showGenresView();
    void onSearchHideButtonClicked();
    void onSearchTextChanged();
#ifdef MAFW
    void browseAutomaticPlaylists(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error);
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllArtists(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllAlbums(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllGenres(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onAddToNowPlayingCallback(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString error);
    void onShareUriReceived(QString, QString uri);
    void onRingingToneUriReceived(QString objectId, QString uri);
    void listSongs();
    void listArtists();
    void listAlbums();
    void listGenres();
    void listPlaylists();
    void listAutoPlaylists();
    void listImportedPlaylists();
    void onAlbumSelected(QModelIndex index);
    void onArtistSelected(QModelIndex index);
    void onGenreSelected(QModelIndex index);
    void onPlaylistSelected(QModelIndex index);
    void onGetItems(QString objectId, GHashTable*, guint index, gpointer op);
    void onContainerChanged(QString objectId);
#endif
    void onAddToNowPlaying();
    void onAddToPlaylist();
    void onRenamePlaylist();
    void onRenamePlaylistAccepted();
    void onDeletePlaylistClicked();
    void onNowPlayingWindowHidden();
    void onChildClosed();
};

#endif // MUSICWINDOW_H
