#ifndef SINGLEGENREVIEW_H
#define SINGLEGENREVIEW_H

#include <QMainWindow>

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "delegates/artistlistitemdelegate.h"
#include "delegates/shufflebuttondelegate.h"
#include "includes.h"
#include "ui_singlegenreview.h"
#include "singleartistview.h"
#include "nowplayingwindow.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include <QMaemo5ValueButton>
#endif

namespace Ui {
    class SingleGenreView;
}

class SingleGenreView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleGenreView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleGenreView();
    bool eventFilter(QObject *, QEvent *e);
    void browseGenre(QString objectId);

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

private:
    Ui::SingleGenreView *ui;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseGenreId;
    uint addToNowPlayingId;
    QString currentObjectId;
    gchar** songAddBuffer;
    int songAddBufferSize;
    int visibleSongs;
    bool isShuffling;
#endif
#ifdef Q_WS_MAEMO_5
    void notifyOnAddedToNowPlaying(int songCount);
#endif
    void updateSongCount();

private slots:
    void orientationChanged(int w, int h);
    void onItemActivated(QListWidgetItem *item);
    void onSearchHideButtonClicked();
    void onSearchTextChanged(QString text);
    void addAllToNowPlaying();
    void onContextMenuRequested(const QPoint &pos = QPoint(35,35));
    void showWindowMenu();
    void addArtistToNowPlaying();
    void onNowPlayingWindowHidden();
    void onChildClosed();
#ifdef MAFW
    void listArtists();
    void browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString);
    void onNowPlayingBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable*,QString);
    void onContainerChanged(QString objectId);
#endif
};

#endif // SINGLEGENREVIEW_H
