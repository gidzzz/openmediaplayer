#ifndef VIDEOSWINDOW_H
#define VIDEOSWINDOW_H

#include "browserwindow.h"

#include <QActionGroup>
#include <QAction>
#include <QSettings>

#include "confirmdialog.h"
#include "metadatadialog.h"
#include "videonowplayingwindow.h"
#include "delegates/thumbnailitemdelegate.h"
#include "delegates/mediawithicondelegate.h"

#include "mafw/mafwregistryadapter.h"
#include <libmafw/mafw-source.h>

class VideosWindow : public BrowserWindow
{
    Q_OBJECT

public:
    explicit VideosWindow(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);

private:
    QList<QStandardItem*> recordingsBufferList;
    QList<QStandardItem*> filmsBufferList;

    QAction *sortByDate;
    QAction *sortByCategory;
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwTrackerSource;
    CurrentPlaylistAdapter *playlist;
    unsigned int browseId;

private slots:
    void onSourceReady();
    void onShareClicked();
    void onDeleteClicked();
    void onDetailsClicked();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onVideoSelected(QModelIndex index);
    void onSortingChanged(QAction *action);
    void selectView();
    void listVideos();
    void browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata);
    void onMetadataChanged(QString objectId);
    void onContainerChanged(QString objectId);
};

#endif // VIDEOSWINDOW_H
