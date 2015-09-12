#ifndef UPNPVIEW_H
#define UPNPVIEW_H

#include "browserwindow.h"

#include "includes.h"

#include "metadatadialog.h"
#include "nowplayingwindow.h"
#include "videonowplayingwindow.h"
#include "delegates/mediawithicondelegate.h"

#include "mafw/mafwregistryadapter.h"

class UpnpView : public BrowserWindow
{
    Q_OBJECT

public:
    explicit UpnpView(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0, MafwSourceAdapter *source = 0);
    ~UpnpView();

public slots:
    void browseObjectId(QString objectId);

private slots:
    void onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString);
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onItemActivated(QModelIndex index);
    void onAddOneToNowPlaying();
    void onAddOneToPlaylist();
    void openCurrentUri();
    void showDetails();
    void addAllToNowPlaying();
    int appendAllToPlaylist(QString type, bool filter);
    void onNowPlayingWindowHidden();

private:
    MafwRegistryAdapter *mafwRegistry;
    MafwSourceAdapter *mafwSource;
    CurrentPlaylistAdapter *playlist;
    uint browseId;

    void notifyOnAddedToNowPlaying(int songCount);
};

#endif // UPNPVIEW_H
