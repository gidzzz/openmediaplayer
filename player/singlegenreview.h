#ifndef SINGLEGENREVIEW_H
#define SINGLEGENREVIEW_H

#include "browserwindow.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "delegates/artistlistitemdelegate.h"
#include "delegates/shufflebuttondelegate.h"
#include "includes.h"
#include "singleartistview.h"
#include "nowplayingwindow.h"
#include "currentplaylistmanager.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif

class SingleGenreView : public BrowserWindow
{
    Q_OBJECT

public:
    explicit SingleGenreView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    void browseGenre(QString objectId);

private:
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseGenreId;
    uint playlistToken;
    QString currentObjectId;
    int visibleSongs;
    bool shuffleRequested;
#endif
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif
    void updateSongCount();

private slots:
    void onItemActivated(QModelIndex index);
    void addAllToNowPlaying();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void addArtistToNowPlaying();
    void onNowPlayingWindowHidden();
#ifdef MAFW
    void listArtists();
    void browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString);
    void onAddFinished(uint browseId, int count);
    void onContainerChanged(QString objectId);
#endif
};

#endif // SINGLEGENREVIEW_H
