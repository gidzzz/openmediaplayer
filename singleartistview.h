#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include <QMainWindow>

#ifdef MAFW
    #include "mafwrendereradapter.h"
    #include "mafwsourceadapter.h"
    #include "mafwplaylistadapter.h"
#endif

namespace Ui {
    class SingleArtistView;
}

class SingleArtistView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0, MafwPlaylistAdapter* pls = 0);
    ~SingleArtistView();

private:
    Ui::SingleArtistView *ui;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
#endif
};

#endif // SINGLEARTISTVIEW_H
