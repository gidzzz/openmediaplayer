#ifndef UPNPVIEW_H
#define UPNPVIEW_H

#include "basewindow.h"

#include "ui_upnpview.h"
#include "includes.h"

#include "nowplayingwindow.h"
#include "videonowplayingwindow.h"
#include "delegates/mediawithicondelegate.h"


#include "mafw/mafwadapterfactory.h"

namespace Ui {
    class UpnpView;
}

class UpnpView : public BaseWindow
{
    Q_OBJECT

public:
    explicit UpnpView(QWidget *parent = 0, MafwAdapterFactory *factory = 0, MafwSourceAdapter *source = 0);
    ~UpnpView();
    bool eventFilter(QObject *, QEvent *e);

public slots:
    void browseObjectId(QString objectId);

private slots:
    void onOrientationChanged(int w, int h);
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString text);
    void onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString);
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onItemActivated(QModelIndex index);
    void onAddOneToNowPlaying();
    void onAddOneToPlaylist();
    void addAllToNowPlaying();
    int appendAllToPlaylist(QString type);
    void onNowPlayingWindowHidden();
    void onChildClosed();

private:
    Ui::UpnpView *ui;

    QStandardItemModel *objectModel;
    QSortFilterProxyModel *objectProxyModel;

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    void notifyOnAddedToNowPlaying(int songCount);
    uint browseId;
    MafwAdapterFactory *mafwFactory;
    MafwSourceAdapter *mafwSource;
    MafwPlaylistAdapter* playlist;
};

#endif // UPNPVIEW_H
