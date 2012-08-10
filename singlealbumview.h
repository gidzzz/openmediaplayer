#ifndef SINGLEALBUMVIEW_H
#define SINGLEALBUMVIEW_H

#include <QMainWindow>
#include <QTime>

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5ValueButton>
#else
    #include <QPushButton>
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "ui_singlealbumview.h"
#include "includes.h"
#include "confirmdialog.h"
#include "nowplayingwindow.h"
#include "delegates/singlealbumviewdelegate.h"
#include "delegates/shufflebuttondelegate.h"


namespace Ui {
    class SingleAlbumView;
}

class SingleAlbumView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleAlbumView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleAlbumView();
    bool eventFilter(QObject *, QEvent *e);
#ifdef MAFW
    void browseAlbumByObjectId(QString objectId);
#endif

private:
    Ui::SingleAlbumView *ui;
    QString albumObjectId;
    int visibleSongs;
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseAllSongsId;
#endif
    void updateSongCount();

private slots:
    void orientationChanged(int w, int h);
#ifdef MAFW
    void listSongs();
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onItemActivated(QListWidgetItem *item);
    void onRingingToneUriReceived(QString objectId, QString uri);
    void onShareUriReceived(QString objectId, QString uri);
    void onContainerChanged(QString objectId);
#endif
    int appendAllToPlaylist(bool filter);
    void playAll(int startIndex);
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString);
    void addAllToNowPlaying();
    void deleteCurrentAlbum();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void showWindowMenu();
    void setRingingTone();
    void onShareClicked();
    void onDeleteClicked();
    void onAddToNowPlaying();
    void onAddToPlaylist();
    void onNowPlayingWindowHidden();
};

#endif // SINGLEALBUMVIEW_H
