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
#include "singlealbumview.h"
#include "singleartistview.h"
#include "ui_musicwindow.h"
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include "mafwrendereradapter.h"
    #include "mafwsourceadapter.h"
    #include "mafwplaylistadapter.h"
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
    explicit MusicWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0, MafwPlaylistAdapter* pls = 0);
    ~MusicWindow();

private:
    Ui::MusicWindow *ui;
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    NowPlayingWindow *myNowPlayingWindow;
    QMenu *contextMenu;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseAllSongsId;
    unsigned int browseAllArtistsId;
    unsigned int browseAllAlbumsId;
    void fetchUri(QString objectId);
#endif
    void connectSignals();
    void populateMenuBar();
    void hideLayoutContents();
    void saveViewState(QVariant);
    void loadViewState();
    QListWidget* currentList();

private slots:
    void onContextMenuRequested(const QPoint&);
    void onSongSelected(QListWidgetItem *);
    void setRingingTone();
    void onShareClicked();
    void onDeleteClicked();
    void orientationChanged();
    void showAlbumView();
    void showPlayListView();
    void showArtistView();
    void showSongsView();
    void showGenresView();
    void onSearchTextChanged(QString);
#ifdef MAFW
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllArtists(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllAlbums(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onDeleteUriReceived(QString objectId, QString uri);
    void onShareUriReceived(QString, QString Uri);
    void onRingingToneUriReceived(QString objectId, QString uri);
    void listSongs();
    void listArtists();
    void listAlbums();
    void onAlbumSelected(QListWidgetItem*);
    void onArtistSelected(QListWidgetItem*);
#endif
};

#endif // MUSICWINDOW_H
