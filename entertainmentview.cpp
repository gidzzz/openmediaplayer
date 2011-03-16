#include "entertainmentview.h"

#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif

EntertainmentView::EntertainmentView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EntertainmentView)
{
    ui->setupUi(this);
    ui->declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    setAttribute(Qt::WA_Maemo5NonComposited);
    QGLWidget *glWidget = new QGLWidget;
    ui->declarativeView->setViewport(glWidget);

    rootObject = dynamic_cast<QObject*>(ui->declarativeView->rootObject());
    rootObject->setParent(this);

    connect(rootObject, SIGNAL(quitButtonClicked()), this, SLOT(close()));
    connect(this, SIGNAL(titleChanged(QVariant)), rootObject, SLOT(setSongTitle(QVariant)));
    connect(this, SIGNAL(albumChanged(QVariant)), rootObject, SLOT(setSongAlbum(QVariant)));
    connect(this, SIGNAL(artistChanged(QVariant)), rootObject, SLOT(setSongArtist(QVariant)));
    connect(this, SIGNAL(albumArtChanged(QVariant)), rootObject, SLOT(setAlbumArt(QVariant)));

#ifdef Q_WS_MAEMO_5
    this->setDNDAtom(true);
#endif
}

EntertainmentView::~EntertainmentView()
{
    delete ui;
}

void EntertainmentView::setMetadata(QString songName, QString albumName, QString artistName, QString albumArtUri)
{
    this->title = songName;
    this->album = albumName;
    this->artist = artistName;
    this->albumArt = albumArtUri;

    emit titleChanged(this->title);
    emit albumChanged(this->album);
    emit artistChanged(this->artist);
    emit albumArtChanged(this->albumArt);
}

#ifdef Q_WS_MAEMO_5
void EntertainmentView::setDNDAtom(bool dnd)
{
    quint32 enable;
    if (dnd)
        enable = 1;
    else
        enable = 0;
    Atom winDNDAtom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);
    XChangeProperty(QX11Info::display(), winId(), winDNDAtom, XA_INTEGER, 32, PropModeReplace, (uchar*) &enable, 1);
}
#endif
