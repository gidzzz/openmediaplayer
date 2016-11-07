#include "qmlview.h"

#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

QmlView::QmlView(QUrl source, QWidget *parent, MafwRegistryAdapter *mafwRegistry ) :
    QMainWindow(parent),
    ui(new Ui::QmlView),
    mafwRegistry(mafwRegistry),
    mafwRenderer(mafwRegistry->renderer())
{
    ui->setupUi(this);
    ui->declarativeView->setSource(source);
    ui->declarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5NonComposited);

    QGLWidget *glWidget = new QGLWidget(this);
    ui->declarativeView->setViewport(glWidget);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    fmtx = new FMTXInterface(this);

    Rotator *rotator = Rotator::acquire();
    savedPolicy = rotator->policy();
    rotator->setPolicy(Rotator::Landscape);

    rootObject = dynamic_cast<QObject*>(ui->declarativeView->rootObject());
    rootObject->setParent(this);

    connect(rootObject, SIGNAL(quitButtonClicked()), this, SLOT(close()));
    connect(rootObject, SIGNAL(prevButtonClicked()), mafwRenderer, SLOT(previous()));
    connect(rootObject, SIGNAL(playButtonClicked()), this, SLOT(onPlayClicked()));
    connect(rootObject, SIGNAL(nextButtonClicked()), mafwRenderer, SLOT(next()));
    connect(rootObject, SIGNAL(fmtxButtonClicked()), this, SLOT(onFmtxClicked()));
    connect(rootObject, SIGNAL(sliderValueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(rootObject, SIGNAL(playlistItemSelected(int)), this, SLOT(onPlaylistItemChanged(int)));

    connect(this, SIGNAL(titleChanged(QVariant)), rootObject, SLOT(setSongTitle(QVariant)));
    connect(this, SIGNAL(albumChanged(QVariant)), rootObject, SLOT(setSongAlbum(QVariant)));
    connect(this, SIGNAL(artistChanged(QVariant)), rootObject, SLOT(setSongArtist(QVariant)));
    connect(this, SIGNAL(albumArtChanged(QVariant)), rootObject, SLOT(setAlbumArt(QVariant)));
    connect(this, SIGNAL(durationTextChanged(QVariant)), rootObject, SLOT(setPosition(QVariant)));
    connect(this, SIGNAL(positionChanged(QVariant)), rootObject, SLOT(setSliderValue(QVariant)));
    connect(this, SIGNAL(durationChanged(QVariant)), rootObject, SLOT(setSliderMaximum(QVariant)));
    connect(this, SIGNAL(stateIconChanged(QVariant)), rootObject, SLOT(setPlayButtonIcon(QVariant)));
    connect(this, SIGNAL(rowChanged(QVariant)), rootObject, SLOT(onRowChanged(QVariant)));
    connect(this, SIGNAL(fmtxStateChanged(QVariant)), rootObject, SLOT(onFmtxStateChanged(QVariant)));

    connect(this, SIGNAL(playlistItemAppended(QVariant,QVariant,QVariant)),
            rootObject, SLOT(appendPlaylistItem(QVariant,QVariant,QVariant)));
    connect(this, SIGNAL(playlistItemInserted(QVariant,QVariant,QVariant,QVariant)),
            rootObject, SLOT(insertPlaylistItem(QVariant,QVariant,QVariant,QVariant)));
    connect(this, SIGNAL(playlistItemSet(QVariant,QVariant,QVariant,QVariant)),
            rootObject, SLOT(setPlaylistItem(QVariant,QVariant,QVariant,QVariant)));
    connect(this, SIGNAL(playlistItemRemoved(QVariant)),
            rootObject, SLOT(removePlaylistItem(QVariant)));
    connect(this, SIGNAL(playlistCleared()),
            rootObject, SLOT(clearPlaylist()));

    connect(mafwRenderer, SIGNAL(stateChanged(MafwPlayState)), this, SLOT(onStateChanged(MafwPlayState)));
    connect(mafwRenderer, SIGNAL(positionReceived(int,QString)), this, SLOT(onPositionChanged(int)));
    connect(mafwRenderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState)));
    connect(positionTimer, SIGNAL(timeout()), mafwRenderer, SLOT(getPosition()));

    connect(fmtx, SIGNAL(propertyChanged()), this, SLOT(onFmtxChanged()));
    onFmtxChanged();

    positionTimer->start();

    quint32 disable = {0};
    Atom winPortraitModeSupportAtom = XInternAtom(QX11Info::display(), "_HILDON_PORTRAIT_MODE_SUPPORT", false);
    XChangeProperty(QX11Info::display(), winId(), winPortraitModeSupportAtom, XA_CARDINAL, 32, PropModeReplace, (uchar*) &disable, 1);

    this->setDNDAtom(true);

    mafwRenderer->getStatus();
    mafwRenderer->getPosition();
}

QmlView::~QmlView()
{
    Rotator::acquire()->setPolicy(savedPolicy);

    delete ui;
}

void QmlView::setMetadata(QString songName, QString albumName, QString artistName, QString albumArtUri, int duration)
{
    this->title = songName;
    this->album = albumName;
    this->artist = artistName;
    this->albumArt = albumArtUri;
    this->duration = duration;
    this->songDuration = duration;

    emit titleChanged(this->title);
    emit albumChanged(this->album);
    emit artistChanged(this->artist);
    emit albumArtChanged(this->albumArt);
    emit durationChanged(this->duration);
}

void QmlView::setDNDAtom(bool dnd)
{
    quint32 enable = dnd ? 1 : 0;
    Atom winDNDAtom = XInternAtom(QX11Info::display(), "_HILDON_DO_NOT_DISTURB", false);
    XChangeProperty(QX11Info::display(), winId(), winDNDAtom, XA_INTEGER, 32, PropModeReplace, (uchar*) &enable, 1);
}

void QmlView::onFmtxChanged()
{
    emit fmtxStateChanged(fmtx->state() == FMTXInterface::Enabled ? "enabled" : "disabled");
}

void QmlView::onPositionChanged(int position)
{
    duration = mmss_pos(position) + "/" + mmss_len(songDuration);
    emit durationTextChanged(duration);
    emit positionChanged(position);
}

void QmlView::onStateChanged(MafwPlayState state)
{
    this->mafwState = state;
    QString playButtonIconString;

    if (state == Paused) {
        playButtonIconString = playButtonIcon;
        /*ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(resume()));*/
        mafwRenderer->getPosition();
        if (positionTimer->isActive())
            positionTimer->stop();
    }
    else if (state == Playing) {
        playButtonIconString = QString(pauseButtonIcon).prepend("file://");
        /*ui->playButton->setIcon(QIcon(pauseButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(pause()));*/
        mafwRenderer->getPosition();
        if (!positionTimer->isActive())
            positionTimer->start();
    }
    else if (state == Stopped) {
        playButtonIconString = QString(playButtonIcon).prepend("file://");
        /*ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwRenderer, SLOT(play()));*/
        if (positionTimer->isActive())
            positionTimer->stop();
    }
    else if (state == Transitioning) {
        /*ui->songProgress->setEnabled(false);
        ui->songProgress->setValue(0);
        ui->songProgress->setRange(0, 99);
        ui->currentPositionLabel->setText(mmss_pos(0));*/
    }
    emit stateIconChanged(playButtonIconString);
}

void QmlView::onStatusReceived(MafwPlaylist *, uint, MafwPlayState state)
{
    onStateChanged(state);
}

void QmlView::onPlayClicked()
{
    if (this->mafwState == Playing)
        mafwRenderer->pause();
    else if (this->mafwState == Stopped)
        mafwRenderer->play();
    else if (this->mafwState == Paused)
        mafwRenderer->resume();
}

void QmlView::onSliderValueChanged(int position)
{
    mafwRenderer->setPosition(SeekAbsolute, position);
}

void QmlView::appendPlaylistItem(QListWidgetItem *item)
{
    emit playlistItemAppended(item->data(UserRoleSongTitle).toString(),
                              QVariant(item->data(UserRoleSongArtist).toString() + " / " + item->data(UserRoleSongAlbum).toString()),
                              mmss_len(item->data(UserRoleSongDuration).toInt()));
}

void QmlView::insertPlaylistItem(int index, QListWidgetItem *item)
{
    emit playlistItemInserted(index,
                              item->data(UserRoleSongTitle).toString(),
                              QVariant(item->data(UserRoleSongArtist).toString() + " / " + item->data(UserRoleSongAlbum).toString()),
                              mmss_len(item->data(UserRoleSongDuration).toInt()));
}

void QmlView::setPlaylistItem(int index, QListWidgetItem *item)
{
    emit playlistItemSet(index,
                              item->data(UserRoleSongTitle).toString(),
                              QVariant(item->data(UserRoleSongArtist).toString() + " / " + item->data(UserRoleSongAlbum).toString()),
                              mmss_len(item->data(UserRoleSongDuration).toInt()));
}

void QmlView::removePlaylistItem(int index)
{
    emit playlistItemRemoved(index);
}

void QmlView::clearPlaylist()
{
    emit playlistCleared();
}

void QmlView::onPlaylistItemChanged(int index)
{
    mafwRenderer->gotoIndex(index);
}

void QmlView::onFmtxClicked()
{
    fmtx->setEnabled(fmtx->state() != FMTXInterface::Enabled);
}

void QmlView::setCurrentRow(int row)
{
    emit rowChanged(row);
}
