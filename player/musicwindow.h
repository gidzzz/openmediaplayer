#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include "basewindow.h"

#include <QDir>
#include <QStringList>
#include <QDirIterator>
#include <QtGui>

#include "ui_musicwindow.h"
#include "headerawareproxymodel.h"
#include "confirmdialog.h"
#include "nowplayingwindow.h"
#include "ringtonedialog.h"
#include "sharedialog.h"
#include "delegates/songlistitemdelegate.h"
#include "delegates/artistlistitemdelegate.h"
#include "delegates/thumbnailitemdelegate.h"
#include "singlealbumview.h"
#include "singleartistview.h"
#include "singleplaylistview.h"
#include "singlegenreview.h"
#include "ui_musicwindow.h"
#include "includes.h"

#include "mafw/mafwregistryadapter.h"
#include "mafw/mafwplaylistmanageradapter.h"

namespace Ui {
    class MusicWindow;
}

class MusicWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit MusicWindow(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);
    ~MusicWindow();
    bool eventFilter(QObject *obj, QEvent *e);
    void refreshPlaylistView();

signals:
    void hidden();

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

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *);

    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    CurrentPlaylistAdapter *playlist;
    unsigned int browseAllSongsId;
    unsigned int browseAllArtistsId;
    unsigned int browseAllAlbumsId;
    unsigned int browseAllGenresId;
    unsigned int browseRecentlyAddedId;
    unsigned int browseRecentlyPlayedId;
    unsigned int browseMostPlayedId;
    unsigned int browseNeverPlayedId;
    unsigned int browseImportedPlaylistsId;
    unsigned int addToNowPlayingId;
    uint playlistToken;
    int savedPlaylistCount;
    void (MusicWindow::*listAction)();
    void connectSignals();
    void showChild(QMainWindow *window);
    void showView(QListView *listView, void (MusicWindow::*listAction)(), QString name, QString title);
    void loadViewState();
    QListView* currentList();
    void notifyOnAddedToNowPlaying(int songCount);

private slots:
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onRingtoneClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onOrientationChanged(int w, int h);
    void showAlbumView();
    void showPlayListView();
    void showArtistView();
    void showSongsView();
    void showGenresView();
    void onSearchRequested();
    void onSearchHideButtonClicked();
    void onSearchTextChanged();

    void browseSourcePlaylists(uint browseId, int remainingCount, uint index, QString, GHashTable* metadata, QString error);
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllArtists(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllAlbums(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllGenres(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);

    void onAddFinished(uint token, int count);


    void listSongs();
    void listArtists();
    void listAlbums();
    void listGenres();
    void listPlaylists();
    void listAutoPlaylists();
    void listSavedPlaylists();
    void listImportedPlaylists();

    void onSongSelected(QModelIndex index);
    void onAlbumSelected(QModelIndex index);
    void onArtistSelected(QModelIndex index);
    void onGenreSelected(QModelIndex index);
    void onPlaylistSelected(QModelIndex index);

    void onContainerChanged(QString objectId);

    void onAddToNowPlaying();
    void onAddToPlaylist();
    void onRenamePlaylist();
    void onRenamePlaylistAccepted();
    void onDeletePlaylistClicked();
    void onNowPlayingWindowHidden();
    void onChildClosed();
};

#endif // MUSICWINDOW_H
