#ifndef UPNPVIEW_H
#define UPNPVIEW_H

#include <QMainWindow>

#include "ui_upnpview.h"
#include "includes.h"

#include "nowplayingwindow.h"
#include "videonowplayingwindow.h"
#include "delegates/mediawithicondelegate.h"


#include "mafw/mafwadapterfactory.h"

namespace Ui {
    class UpnpView;
}

class UpnpView : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpnpView(QWidget *parent = 0, MafwAdapterFactory *factory = 0, MafwSourceAdapter *source = 0);
    ~UpnpView();

public slots:
    void browseObjectId(QString objectId);

private slots:
    void onOrientationChanged();
    void onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString);
    void onContextMenuRequested(const QPoint &point);
    void onItemActivated(QListWidgetItem *item);
    void onAddOneToNowPlaying();
    void addAllToNowPlaying();
    int appendAllToPlaylist();
    void onNowPlayingWindowHidden();
    void onChildClosed();

private:
    void notifyOnAddedToNowPlaying(int songCount);
    Ui::UpnpView *ui;
    uint browseId;
    MafwAdapterFactory *mafwFactory;
    MafwSourceAdapter *mafwSource;
    MafwPlaylistAdapter* playlist;
};

#endif // UPNPVIEW_H
