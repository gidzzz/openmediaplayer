#ifndef SINGLEPLAYLISTVIEW_H
#define SINGLEPLAYLISTVIEW_H

#include "browserwindow.h"

#include "confirmdialog.h"
#include "playlistquerymanager.h"
#include "delegates/songlistitemdelegate.h"
#include "delegates/shufflebuttondelegate.h"

#include <QMaemo5InformationBox>

#include "mafw/mafwregistryadapter.h"

class SinglePlaylistView : public BrowserWindow
{
    Q_OBJECT

    enum PendingActivation
    {
        Nothing = -1,
        AddToNowPlaying = -2,
        AddToPlaylist = -3
    };

public:
    explicit SinglePlaylistView(QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);
    bool eventFilter(QObject *obj, QEvent *e);
    void browseAutomaticPlaylist(QString filter, QString sorting, int maxCount);
    void browseSavedPlaylist(MafwPlaylist *mafwplaylist);
    void browseImportedPlaylist(QString objectId);

protected:
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *e);

private:
    QTimer *clickTimer;
    QModelIndex clickedIndex;
    bool permanentDelete;
    bool playlistModified;
    bool playlistLoaded;
    int pendingActivation;
    void notifyOnAddedToNowPlaying(int songCount);

    QString currentObjectId;
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    CurrentPlaylistAdapter *playlist;
    uint browsePlaylistId;
    int remainingCount;

private slots:
    void updateSongCount();
    void setItemMetadata(QStandardItem *item, QString objectId, GHashTable *metadata);
    void onItemReceived(QString objectId, GHashTable* metadata, uint index);
    void onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString);
    int appendAllToPlaylist(bool filter);
    void onItemActivated(QModelIndex index);
    void addAllToNowPlaying();
    void addAllToPlaylist();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void onAddToNowPlaying();
    void onAddToPlaylist();
    void onRingtoneClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onDeleteFromPlaylist();
    void forgetClick();
    void onItemDoubleClicked();
    void saveCurrentPlaylist();
    void deletePlaylist();
    void onNowPlayingWindowHidden();
};

#endif // SINGLEPLAYLISTVIEW_H
