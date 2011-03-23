#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include <QMainWindow>
#include <QListWidgetItem>

#ifdef MAFW
    #include "mafwrendereradapter.h"
    #include "mafwsourceadapter.h"
    #include "mafwplaylistadapter.h"
#endif

#include "singlealbumview.h"
#include "delegates/thumbnailitemdelegate.h"
#include "includes.h"

namespace Ui {
    class SingleArtistView;
}

class SingleArtistView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0, MafwPlaylistAdapter* pls = 0);
    ~SingleArtistView();
    void browseAlbum(QString artistId);
    void setSongCount(int songCount);

private:
    Ui::SingleArtistView *ui;
    void keyReleaseEvent(QKeyEvent *);
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    uint browseAllAlbumsId;
    QString artistObjectId;
    void listAlbums();
#endif

private slots:
#ifdef MAFW
    void browseAllAlbums(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error);
#endif
    void onAlbumSelected(QListWidgetItem*);
    void orientationChanged();
    void onSearchTextChanged(QString);
};

#endif // SINGLEARTISTVIEW_H
