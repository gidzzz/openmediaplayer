/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "nowplayingindicator.h"

NowPlayingIndicator::NowPlayingIndicator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NowPlayingIndicator)
#ifdef MAFW
  ,window(0)
#endif
{
    ready = false; // avoid segfaults on requesting info from the playlist too early
    poked = false;
    inhibited = 0;
    pokeTimer = new QTimer(this);
    pokeTimer->setInterval(333);
    ui->setupUi(this);
    images << QPixmap(idleFrame);
    for (int i = 1; i < 12; i++)
        images << QPixmap("/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator" + QString::number(i) + ".png");
    frame = 0;
    setAttribute(Qt::WA_OpaquePaintEvent);
    timer = new QTimer(this);
    timer->setInterval(100);
    this->stopAnimation();
#ifdef Q_WS_MAEMO_5
    deviceEvents = new Maemo5DeviceEvents(this);
    setAttribute(Qt::WA_Maemo5NonComposited);
#endif
}

NowPlayingIndicator::~NowPlayingIndicator()
{
    delete ui;
}

void NowPlayingIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, images[frame]);
}

void NowPlayingIndicator::connectSignals()
{
#ifdef Q_WS_MAEMO_5
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(deviceEvents, SIGNAL(screenLocked(bool)), this, SLOT(onTkLockChanged(bool)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onPlaylistReady(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(playlist, SIGNAL(contentsChanged(guint, guint, guint)), this, SLOT(autoSetVisibility()));
    connect(playlist, SIGNAL(playlistChanged()), this, SLOT(autoSetVisibility()));
#endif
    connect(timer, SIGNAL(timeout()), this, SLOT(startAnimation()));
    connect(pokeTimer, SIGNAL(timeout()), this, SLOT(onPokeTimeout()));
}

#ifdef MAFW
void NowPlayingIndicator::onMediaChanged(int, char* objectId)
{
    rendererObjectId = QString::fromUtf8(objectId);
}
#endif

#ifdef MAFW
QString NowPlayingIndicator::currentObjectId()
{
    return rendererObjectId;
}
#endif

#ifdef MAFW
void NowPlayingIndicator::onStateChanged(int state)
{
#ifdef DEBUG
    qDebug() << "NowPlayingIndicator::onStateChanged(int)";
#endif
    mafwState = state;
    if(state == Paused || state == Stopped)
        this->stopAnimation();
    else if(state == Playing && !timer->isActive())
        timer->start();
}
#endif

#ifdef Q_WS_MAEMO_5
void NowPlayingIndicator::onTkLockChanged(bool state)
{
    if (state) {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen locked, stopping animation";
#endif
        this->stopAnimation();
    } else {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen unlocked, starting animation";
#endif
        if (!deviceEvents->isScreenLocked() && this->mafwState == Playing) {
            timer->start();
        } else {
#ifdef DEBUG
            qDebug() << "NowPlayingIndicator: Screen locked, animation blocked.";
#endif
            if (timer->isActive())
                timer->stop();
        }
    }
}
#endif

void NowPlayingIndicator::startAnimation()
{
    // Update the widget frame by frame
    if (this->isVisible()) {
        if (frame == 11)
            frame = 1;
        else
            frame++;
        this->update();
    }
}

void NowPlayingIndicator::stopAnimation()
{
    if (timer->isActive())
        timer->stop();
    this->frame = 0;
    this->repaint();
}

void NowPlayingIndicator::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Music"), this, SLOT(onAudioPlaylistSelected()));
    contextMenu->exec(e->globalPos());
}

void NowPlayingIndicator::onAudioPlaylistSelected()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    if (playlist->getSize()) {
        window = NowPlayingWindow::acquire(this->parentWidget(), mafwFactory);
        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        window->show();
        this->inhibit();
    }
#endif
}

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
#ifdef MAFW
        QString playlistName = playlist->playlistName();
        qDebug() << "Current playlist is" << playlistName;


        if (playlistName == "FmpRadioPlaylist" && window == 0)  {
            window = new RadioNowPlayingWindow(this, mafwFactory);
            window->setAttribute(Qt::WA_DeleteOnClose);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
            window->show();
        }
        else if (playlistName == "FmpVideoPlaylist") {
            VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this->parentWidget(), mafwFactory);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
            QTimer::singleShot(500, window, SLOT(playVideo()));
            window->showFullScreen();
        }
        // The user can only create audio playlists with the UX
        // Assume all other playlists are audio ones.
        else { // playlistName == "FmpAudioPlaylist"
            window = NowPlayingWindow::acquire(this->parentWidget(), mafwFactory);
            connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
            window->show();
        }
#else
        NowPlayingWindow *window = NowPlayingWindow::acquire(this);
        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        window->show();
#endif
        this->inhibit();
    }
}

void NowPlayingIndicator::onWindowDestroyed()
{
    this->restore();
    window = 0;
}

void NowPlayingIndicator::onNowPlayingWindowHidden() // could be joined with the method above
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    this->restore();
    window = 0;
}

#ifdef MAFW
void NowPlayingIndicator::onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString)
{
#ifdef DEBUG
    qDebug() << "NowPlayingIndicator::onGetStatus";
#endif
    this->onStateChanged(state);
}
#endif

#ifdef MAFW
void NowPlayingIndicator::onPlaylistReady(MafwPlaylist*, uint, MafwPlayState, const char *objectId, QString)
{
    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onPlaylistReady(MafwPlaylist*,uint,MafwPlayState,const char *,QString)));
    ready = true;
        this->rendererObjectId = objectId;
    this->autoSetVisibility();
}
#endif

void NowPlayingIndicator::showEvent(QShowEvent *)
{
#ifdef MAFW
    mafwrenderer->getStatus();
    if (this->mafwState == Playing && !timer->isActive())
        timer->start();
#endif
}

void NowPlayingIndicator::hideEvent(QHideEvent *)
{
#ifdef Q_WS_MAEMO_5
    this->stopAnimation();
#endif
}

void NowPlayingIndicator::triggerAnimation()
{
#ifdef Q_WS_MAEMO_5
    if (this->mafwState == Playing && !timer->isActive())
        timer->start();
#endif
}

#ifdef MAFW
void NowPlayingIndicator::setFactory(MafwAdapterFactory *factory)
{
    this->mafwFactory = factory;
    this->mafwrenderer = factory->getRenderer();
    this->playlist = factory->getPlaylistAdapter();
    this->connectSignals();
    mafwrenderer->getStatus();
}
#endif

void NowPlayingIndicator::autoSetVisibility()
{
    qDebug() << "NowPlayingIndicator::autoSetVisibility";
    if (inhibited)
        return;

    if (ready && playlist->getSize())
        this->show();
    else
        this->hide();
}

void NowPlayingIndicator::inhibit()
{
    ++inhibited;
    this->hide();
}

void NowPlayingIndicator::restore()
{
    if (inhibited > 1)
        --inhibited;
    else
        inhibited = 0;

    this->autoSetVisibility();
}

void NowPlayingIndicator::poke()
{
    if (!poked) {
        this->inhibit();
        poked = true;
    }
    pokeTimer->start();
}

void NowPlayingIndicator::onPokeTimeout()
{
    this->restore();
    pokeTimer->stop();
    poked = false;
}
