#ifndef SINGLEARTISTVIEW_H
#define SINGLEARTISTVIEW_H

#include <QMainWindow>

#ifdef MAFW
    #include <mafwrendereradapter.h>
    #include <mafwsourceadapter.h>
#endif

namespace Ui {
    class SingleArtistView;
}

class SingleArtistView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SingleArtistView(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0);
    ~SingleArtistView();

private:
    Ui::SingleArtistView *ui;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
#endif
};

#endif // SINGLEARTISTVIEW_H
