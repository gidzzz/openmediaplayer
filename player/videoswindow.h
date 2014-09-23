#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include "browserwindow.h"

#include <QActionGroup>
#include <QAction>
#include <QSettings>

#include "confirmdialog.h"
#include "videonowplayingwindow.h"
#include "delegates/thumbnailitemdelegate.h"
#include "delegates/mediawithicondelegate.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
    #include <libmafw/mafw-source.h>
#endif

class VideosWindow : public BrowserWindow
{
    Q_OBJECT

public:
    explicit VideosWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);

private:
    QList<QStandardItem*> recordingsBufferList;
    QList<QStandardItem*> filmsBufferList;

    QAction *sortByDate;
    QAction *sortByCategory;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    unsigned int browseId;
#endif

private slots:
    void onSourceReady();
    void onShareClicked();
    void onDeleteClicked();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onVideoSelected(QModelIndex index);
    void onSortingChanged(QAction *action);
    void selectView();
#ifdef MAFW
    void listVideos();
    void browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onMetadataChanged(QString objectId);
    void onContainerChanged(QString objectId);
#endif
};

#endif // VIDEOSWINDOW_H
