#include "singleartistview.h"
#include "ui_singleartistview.h"

SingleArtistView::SingleArtistView(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa) :
    QMainWindow(parent),
    ui(new Ui::SingleArtistView)
#ifdef MAFW
    ,mafwTrackerSource(msa),
    mafwrenderer(mra)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
}

SingleArtistView::~SingleArtistView()
{
    delete ui;
}
