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
#include "ui_musicwindow.h"
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include "mafwrendereradapter.h"
    #include "mafwsourceadapter.h"
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
    explicit MusicWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0);
    ~MusicWindow();

public slots:
    void selectSong();

private:
    Ui::MusicWindow *ui;
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    NowPlayingWindow *myNowPlayingWindow;
    QMenu *contextMenu;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
    unsigned int browseAllSongsId;
    unsigned int browseAllArtistsId;
#endif
    void connectSignals();
    void populateMenuBar();
    void hideLayoutContents();
    void saveViewState(QVariant);
    void loadViewState();

private slots:
    void onContextMenuRequested(const QPoint&);
    void onShareClicked();
    void onDeleteClicked();
    void orientationChanged();
    void showAlbumView();
    void showPlayListView();
    void showArtistView();
    void showSongsView();
    void showGenresView();
#ifdef MAFW
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void browseAllArtists(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void listSongs();
    void listArtists();
#endif
};

#endif // MUSICWINDOW_H
