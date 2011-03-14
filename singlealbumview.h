#ifndef SINGLEALBUMVIEW_H
#define SINGLEALBUMVIEW_H

#include <QMainWindow>
#include <QTime>

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5ValueButton>
#else
    #include <QPushButton>
#endif

#ifdef MAFW
    #include "mafwsourceadapter.h"
    #include "mafwrendereradapter.h"
    #include "mafwplaylistadapter.h"
#endif

#include "ui_singlealbumview.h"
#include "includes.h"
#include "nowplayingwindow.h"
#include "delegates/singlealbumviewdelegate.h"


namespace Ui {
    class SingleAlbumView;
}

class SingleAlbumView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleAlbumView(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0, MafwPlaylistAdapter* pls = 0);
    ~SingleAlbumView();
    void browseAlbum(QString);
    void browseSingleAlbum(QString);

private:
    Ui::SingleAlbumView *ui;
    NowPlayingWindow *npWindow;
    QString albumName;
    void keyPressEvent(QKeyEvent *);
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleAllButton;
#else
    QPushButton *shuffleAllButton;
#endif
#ifdef MAFW
    MafwSourceAdapter *mafwTrackerSource;
    MafwRendererAdapter* mafwrenderer;
    MafwPlaylistAdapter* playlist;
    unsigned int browseAllSongsId;
#endif

private slots:
    void orientationChanged();
#ifdef MAFW
    void listSongs();
    void browseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error);
    void onItemSelected(QListWidgetItem*);
#endif
    void createPlaylist(bool);
    void onShuffleButtonClicked();
};

#endif // SINGLEALBUMVIEW_H
