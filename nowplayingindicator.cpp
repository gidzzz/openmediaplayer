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
            this, SLOT(onPlaylistReady()));
    connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
    connect(playlist, SIGNAL(contentsChanged()), this, SLOT(autoSetVisibility()));
#endif
    connect(timer, SIGNAL(timeout()), this, SLOT(startAnimation()));
}

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
    if(state) {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen locked, stopping animation";
#endif
        this->stopAnimation();
    } else {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen unlocked, starting animation";
#endif
        if(!deviceEvents->isScreenLocked() && this->mafwState == Playing) {
            timer->start();
        } else {
#ifdef DEBUG
            qDebug() << "NowPlayingIndicator: Screen locked, animation blocked.";
#endif
            if(timer->isActive())
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
#ifdef DEBUG
    qDebug() << "NowPlayingIndicator::stopAnimation()";
#endif
    if(timer->isActive())
        timer->stop();
    this->frame = 0;
    this->repaint();
}

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        emit clicked();
#ifdef MAFW
        QString playlistName = playlist->playlistName();
#ifdef DEBUG
        qDebug() << "Current playlist is: " + playlistName;
#endif
        if (playlistName == "FmpVideoPlaylist" && window == 0) {
            window = new VideoNowPlayingWindow(this, mafwFactory);
            window->setAttribute(Qt::WA_DeleteOnClose);
            connect(window, SIGNAL(destroyed), this, SLOT(onWindowDestroyed()));
            }
        else if (playlistName == "FmpRadioPlaylist" && window == 0)  {
            window = new RadioNowPlayingWindow(this, mafwFactory);
            window->setAttribute(Qt::WA_DeleteOnClose);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
        }
        // The user can only create audio playlists with the UX
        // Assume all other playlists are audio ones.
        else
            window = NowPlayingWindow::acquire(this->parentWidget(), mafwFactory);
        if (playlistName == "FmpVideoPlaylist")
            window->showFullScreen();
        else //{
            window->show();
        //connect(window, SIGNAL(hidden()), this, SLOT(autoSetVisibility()));
        //this->hide();
        //}
#else
        NowPlayingWindow *window = NowPlayingWindow::acquire(this);
        window->show();
#endif
    }
}

void NowPlayingIndicator::onWindowDestroyed()
{
    window = 0;
    //this->autoSetVisibility();
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
void NowPlayingIndicator::onPlaylistReady()
{
    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
        this, SLOT(onPlaylistReady()));
    ready = true;
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
    if(this->mafwState == Playing)
        if(!timer->isActive())
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
    if (ready && playlist->getSize())
        this->show();
    else
        this->hide();
}
