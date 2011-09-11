#ifndef SINGLEGENREVIEW_H
#define SINGLEGENREVIEW_H

#include <QMainWindow>

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

#include "delegates/artistlistitemdelegate.h"
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

protected:
    void keyReleaseEvent(QKeyEvent *);

public:
    explicit SingleGenreView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~SingleGenreView();
    void browseGenre(QString objectId);
    void setSongCount(int);

private:
    Ui::SingleGenreView *ui;
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseGenreId;
    uint addToNowPlayingId;
    QString objectIdToBrowse;
    QString currentObjectId;
    gchar** songAddBuffer;
    int songAddBufferSize;
    int songsInGenre;
    bool isShuffling;
#endif
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleAllButton;
    void notifyOnAddedToNowPlaying(int songCount);
#else
    QPushButton *shuffleAllButton;
#endif

private slots:
    void orientationChanged();
    void onItemSelected(QListWidgetItem *item);
    void onSearchTextChanged(QString text);
    void addAllToNowPlaying();
    void onShuffleButtonClicked();
    void onContextMenuRequested(QPoint point);
    void addItemToNowPlaying();
#ifdef MAFW
    void listGenres();
    void browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString);
    void onNowPlayingBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable*,QString);
#endif
};

#endif // SINGLEGENREVIEW_H
