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
#include "nowplayingwindow.h"
#include "delegates/singlealbumviewdelegate.h"


namespace Ui {
    class SingleAlbumView;
}

class SingleAlbumView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleAlbumView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleAlbumView();
#ifdef MAFW
    void browseAlbumByObjectId(QString objectId);
    bool isSingleAlbum;
#endif

private:
    Ui::SingleAlbumView *ui;
    NowPlayingWindow *npWindow;
    QString albumName;
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleAllButton;
    void notifyOnAddedToNowPlaying(int songCount);
#else
    QPushButton *shuffleAllButton;
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseAllSongsId;
#endif

private slots:
    void orientationChanged();
#ifdef MAFW
    void listSongs();
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onItemSelected(QListWidgetItem*);
    void onRingingToneUriReceived(QString objectId, QString uri);
    void onShareUriReceived(QString objectId, QString Uri);
    void onDeleteUriReceived(QString objectId, QString uri);
#endif
    void createPlaylist(bool);
    void onShuffleButtonClicked();
    void onSearchTextChanged(QString);
    void addAllToNowPlaying();
    void onContextMenuRequested(const QPoint &point);
    void setRingingTone();
    void onShareClicked();
    void onDeleteClicked();
    void onAddToNowPlaying();
    void onNowPlayingWindowDestroyed();
};

#endif // SINGLEALBUMVIEW_H
