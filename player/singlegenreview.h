#ifndef SINGLEGENREVIEW_H
#define SINGLEGENREVIEW_H

#include "browserwindow.h"

#include "mafw/mafwregistryadapter.h"

#include "delegates/artistlistitemdelegate.h"
#include "delegates/shufflebuttondelegate.h"
#include "includes.h"
#include "metadatadialog.h"
#include "singleartistview.h"
#include "nowplayingwindow.h"
#include "currentplaylistmanager.h"

#include <QMaemo5InformationBox>

class SingleGenreView : public BrowserWindow
{
    Q_OBJECT

public:
    explicit SingleGenreView(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);
    void browseGenre(QString objectId);

private:
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwTrackerSource;
    CurrentPlaylistAdapter *playlist;
    uint browseGenreId;
    uint playlistToken;
    QString currentObjectId;
    int visibleSongs;
    bool shuffleRequested;
    void notifyOnAddedToNowPlaying(int songCount);
    void updateSongCount();

private slots:
    void onItemActivated(QModelIndex index);
    void addAllToNowPlaying();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void addArtistToNowPlaying();
    void showDetails();
    void onNowPlayingWindowHidden();
    void listArtists();
    void browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable *metadata);
    void onAddFinished(uint browseId, int count);
    void onContainerChanged(QString objectId);
};

#endif // SINGLEGENREVIEW_H
