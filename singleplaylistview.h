#ifndef SINGLEPLAYLISTVIEW_H
#define SINGLEPLAYLISTVIEW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "ui_singleplaylistview.h"
#include "confirmdialog.h"
#include "playlistquerymanager.h"
#include "delegates/songlistitemdelegate.h"
#include "delegates/shufflebuttondelegate.h"
#include "headerawareproxymodel.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include <QMaemo5ValueButton>
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

namespace Ui {
    class SinglePlaylistView;
}

class SinglePlaylistView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SinglePlaylistView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SinglePlaylistView();
    bool eventFilter(QObject *, QEvent *e);
#ifdef MAFW
    void browseAutomaticPlaylist(QString filter, QString sorting, int maxCount);
    void browseSavedPlaylist(MafwPlaylist *mafwplaylist);
    void browseImportedPlaylist(QString objectId);
#endif

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
#ifdef MAFW
    void closeEvent(QCloseEvent *e);
#endif

private:
    Ui::SinglePlaylistView *ui;

    QStandardItemModel *songModel;
    QSortFilterProxyModel *songProxyModel;

    QTimer *clickTimer;
    QModelIndex clickedIndex;
    bool permanentDelete;
    bool playlistModified;
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
    void updateSongCount();

private slots:
    void orientationChanged(int w, int h);
#ifdef MAFW
    void setItemMetadata(QStandardItem *item, QString objectId, GHashTable *metadata);
    void onGetItems(QString objectId, GHashTable* metadata, guint index);
    void onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString);
    void onShareUriReceived(QString objectId, QString uri);
    void onRingingToneUriReceived(QString objectId, QString uri);
#endif
    int appendAllToPlaylist(bool filter);
    void onItemActivated(QModelIndex index);
    void addAllToNowPlaying();
    void addAllToPlaylist();
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString text);
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void showWindowMenu();
    void onAddToNowPlaying();
    void onAddToPlaylist();
    void setRingingTone();
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
