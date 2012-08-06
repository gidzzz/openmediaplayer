#ifndef SINGLEPLAYLISTVIEW_H
#define SINGLEPLAYLISTVIEW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "ui_singleplaylistview.h"
#include "delegates/songlistitemdelegate.h"

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
    QTimer *clickTimer;
    QListWidgetItem *clickedItem;
    bool permanentDelete;
    bool playlistModified;
    int visibleSongs;
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleButton;
    void notifyOnAddedToNowPlaying(int songCount);
#else
    QPushButton *shuffleButton;
#endif
    QListWidgetItem* copyItem(QListWidgetItem *item, int index);

#ifdef MAFW
    QString objectId;
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browsePlaylistId;
    gpointer browsePlaylistOp;
    int numberOfSongsToAdd;
#endif
    void setupShuffleButton();
    void updateSongCount();

private slots:
    void orientationChanged(int w, int h);
#ifdef MAFW
    void onGetItems(QString objectId, GHashTable* metadata, guint index, gpointer op);
    void onBrowseResult(uint browseId, int, uint, QString objectId, GHashTable *metadata, QString);
    void onShareUriReceived(QString objectId, QString uri);
    void onRingingToneUriReceived(QString objectId, QString uri);
#endif
    int appendAllToPlaylist(bool filter);
    void playAll(int startIndex, bool filter);
    void onItemActivated(QListWidgetItem *item);
    void addAllToNowPlaying();
    void addAllToPlaylist();
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString text);
    void onShuffleButtonClicked();
    void onContextMenuRequested(const QPoint &pos);
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
