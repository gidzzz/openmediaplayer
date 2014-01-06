#ifndef SINGLEPLAYLISTVIEW_H
#define SINGLEPLAYLISTVIEW_H

#include "browserwindow.h"

#include "confirmdialog.h"
#include "playlistquerymanager.h"
#include "delegates/songlistitemdelegate.h"
#include "delegates/shufflebuttondelegate.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

class SinglePlaylistView : public BrowserWindow
{
    Q_OBJECT

    enum PendingActivation {
        Nothing = -1,
        AddToNowPlaying = -2,
        AddToPlaylist = -3
    };

public:
    explicit SinglePlaylistView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    bool eventFilter(QObject *obj, QEvent *e);
#ifdef MAFW
    void browseAutomaticPlaylist(QString filter, QString sorting, int maxCount);
    void browseSavedPlaylist(MafwPlaylist *mafwplaylist);
    void browseImportedPlaylist(QString objectId);
#endif

protected:
    void keyPressEvent(QKeyEvent *e);
#ifdef MAFW
    void closeEvent(QCloseEvent *e);
#endif

private:
    QTimer *clickTimer;
    QModelIndex clickedIndex;
    bool permanentDelete;
    bool playlistModified;
    bool playlistLoaded;
    int pendingActivation;
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif

#ifdef MAFW
    QString currentObjectId;
    PlaylistQueryManager *playlistQM;
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browsePlaylistId;
    int remainingCount;
#endif

private slots:
    void updateSongCount();
#ifdef MAFW
    void setItemMetadata(QStandardItem *item, QString objectId, GHashTable *metadata);
    void onGetItems(QString objectId, GHashTable* metadata, guint index);
    void onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString);
#endif
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
