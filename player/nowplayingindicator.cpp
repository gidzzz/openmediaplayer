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
    ui(new Ui::NowPlayingIndicator),
    parentToReenable(0),
    window(0)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_Maemo5NonComposited);


    ready = false; // avoid segfaults on requesting info from the playlist too early
    poked = false;
    inhibited = 0;

    pokeTimer = new QTimer(this);
    pokeTimer->setInterval(333);

    animationTimer = new QTimer(this);
    animationTimer->setInterval(100);

    images.reserve(13);
    images << QPixmap("/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator_pause.png");
    for (int i = 1; i <= 12; i++)
        images << QPixmap("/usr/share/icons/hicolor/scalable/hildon/mediaplayer_nowplaying_indicator" + QString::number(i) + ".png");
    frame = 0;

    stopAnimation();
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
    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwRenderer, SLOT(next()));
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwRenderer, SLOT(previous()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this), SIGNAL(activated()), this, SLOT(openWindow()));

    connect(animationTimer, SIGNAL(timeout()), this, SLOT(nextFrame()));
    connect(pokeTimer, SIGNAL(timeout()), this, SLOT(onPokeTimeout()));
    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onTkLockChanged(bool)));

    connect(playlist, SIGNAL(contentsChanged(uint,uint,uint)), this, SLOT(autoSetVisibility()));

    connect(mafwRenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onPlaylistReady(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
}

void NowPlayingIndicator::onStateChanged(int state)
{
    mafwState = state;

    if (state == Paused || state == Stopped)
        stopAnimation();
    else
        triggerAnimation();
}

void NowPlayingIndicator::onTkLockChanged(bool state)
{
    if (state) {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen locked, stopping animation";
#endif
        stopAnimation();
    } else {
#ifdef DEBUG
        qDebug() << "NowPlayingIndicator: Screen unlocked, triggering animation";
#endif
        triggerAnimation();
    }
}

void NowPlayingIndicator::nextFrame()
{
    // Update the widget frame by frame
    if (frame == 12)
        frame = 1;
    else
        frame++;

    this->update();
}

void NowPlayingIndicator::stopAnimation()
{
    animationTimer->stop();
    frame = 0;
    this->update();
}

void NowPlayingIndicator::triggerAnimation()
{
    if (this->isVisible()
    && mafwState == Playing
    && !Maemo5DeviceEvents::acquire()->isScreenLocked()
    && !animationTimer->isActive())
        animationTimer->start();
}

void NowPlayingIndicator::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Music"), this, SLOT(onAudioPlaylistSelected()));
    contextMenu->exec(e->globalPos());
}

void NowPlayingIndicator::onAudioPlaylistSelected()
{
    playlist->assignAudioPlaylist();

    if (playlist->size())
        openWindow();
}

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && this->rect().contains(e->pos()))
        openWindow();
}

void NowPlayingIndicator::openWindow()
{
    QString playlistName = playlist->name();
    qDebug() << "Current playlist is" << playlistName;

    if (window == 0) {
        QWidget *parentWindow = this->parentWidget();
        while (parentWindow && !qobject_cast<QMainWindow*>(parentWindow))
            parentWindow = parentWindow->parentWidget();

        if (parentWindow && parentWindow->isEnabled()) {
            parentToReenable = parentWindow;
            parentToReenable->setEnabled(false);
        }

        if (playlistName == "FmpRadioPlaylist")  {
            window = new RadioNowPlayingWindow(parentWindow, mafwRegistry);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
            window->show();
        }
        else if (playlistName == "FmpVideoPlaylist") {
            window = new VideoNowPlayingWindow(parentWindow, mafwRegistry, true);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
            window->showFullScreen();
        }
        // The user can only create audio playlists with the UX
        // Assume all other playlists are audio ones.
        else { // playlistName == "FmpAudioPlaylist"
            window = NowPlayingWindow::acquire(parentWindow, mafwRegistry);
            connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
            window->show();
        }

        inhibit();
    }
}

void NowPlayingIndicator::onWindowDestroyed()
{
    restore();
    if (parentToReenable)
        parentToReenable->setEnabled(true);
    parentToReenable = 0;
    window = 0;
}

void NowPlayingIndicator::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    onWindowDestroyed();
}

void NowPlayingIndicator::onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString)
{
    onStateChanged(state);
}

void NowPlayingIndicator::onPlaylistReady(MafwPlaylist*, uint, MafwPlayState, const char *objectId, QString)
{
    disconnect(mafwRenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onPlaylistReady(MafwPlaylist*,uint,MafwPlayState,const char *,QString)));

    ready = true;

    autoSetVisibility();
}

void NowPlayingIndicator::showEvent(QShowEvent *)
{
    mafwRenderer->getStatus();
    triggerAnimation();
}

void NowPlayingIndicator::hideEvent(QHideEvent *)
{
    stopAnimation();
}

void NowPlayingIndicator::setRegistry(MafwRegistryAdapter *mafwRegistry)
{
    this->mafwRegistry = mafwRegistry;
    this->mafwRenderer = mafwRegistry->renderer();
    this->playlist = mafwRegistry->playlist();
    this->connectSignals();
    mafwRenderer->getStatus();
}

void NowPlayingIndicator::autoSetVisibility()
{
    if (inhibited) return;

    if (ready && playlist->size())
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

    autoSetVisibility();
}

void NowPlayingIndicator::poke()
{
    if (!poked) {
        inhibit();
        poked = true;
    }
    pokeTimer->start();
}

void NowPlayingIndicator::onPokeTimeout()
{
    restore();
    pokeTimer->stop();
    poked = false;
}

void NowPlayingIndicator::togglePlayback()
{
    if (playlist->name() == "FmpVideoPlaylist") return;

    if (mafwState == Playing)
        mafwRenderer->pause();
    else if (mafwState == Paused)
        mafwRenderer->resume();
    else if (mafwState == Stopped)
        mafwRenderer->play();
}
