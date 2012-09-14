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
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5NonComposited);

#endif

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
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(next()));
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(previous()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space), this), SIGNAL(activated()), this, SLOT(openWindow()));

    connect(animationTimer, SIGNAL(timeout()), this, SLOT(nextFrame()));
    connect(pokeTimer, SIGNAL(timeout()), this, SLOT(onPokeTimeout()));
#ifdef Q_WS_MAEMO_5
    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onTkLockChanged(bool)));

    connect(playlist, SIGNAL(contentsChanged(guint, guint, guint)), this, SLOT(autoSetVisibility()));
    connect(playlist, SIGNAL(playlistChanged()), this, SLOT(autoSetVisibility()));

    connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onPlaylistReady(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
#endif
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
    mafwState = state;

    if (state == Paused || state == Stopped)
        stopAnimation();
    else
        triggerAnimation();
}
#endif

#ifdef Q_WS_MAEMO_5
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
#endif

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
#ifdef MAFW
    if (this->isVisible()
    && mafwState == Playing
    && !Maemo5DeviceEvents::acquire()->isScreenLocked()
    && !animationTimer->isActive())
        animationTimer->start();
#endif
}

void NowPlayingIndicator::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Music"), this, SLOT(onAudioPlaylistSelected()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), contextMenu), SIGNAL(activated()), contextMenu, SLOT(close()));
    contextMenu->exec(e->globalPos());
}

void NowPlayingIndicator::onAudioPlaylistSelected()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    if (playlist->getSize())
        openWindow();
#endif
}

void NowPlayingIndicator::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && this->rect().contains(e->pos()))
        openWindow();
}

void NowPlayingIndicator::openWindow()
{
#ifdef MAFW
    QString playlistName = playlist->playlistName();
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
            window = new RadioNowPlayingWindow(parentWindow, mafwFactory);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
            window->show();
        }
        else if (playlistName == "FmpVideoPlaylist") {
            window = new VideoNowPlayingWindow(parentWindow, mafwFactory, true);
            connect(window, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
            window->showFullScreen();
        }
        // The user can only create audio playlists with the UX
        // Assume all other playlists are audio ones.
        else { // playlistName == "FmpAudioPlaylist"
            window = NowPlayingWindow::acquire(parentWindow, mafwFactory);
            connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
            window->show();
        }

#else
        NowPlayingWindow *window = NowPlayingWindow::acquire(parentWindow);
        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        window->show();
#endif

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

#ifdef MAFW
void NowPlayingIndicator::onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString)
{
    onStateChanged(state);
}
#endif

#ifdef MAFW
void NowPlayingIndicator::onPlaylistReady(MafwPlaylist*, uint, MafwPlayState, const char *objectId, QString)
{
    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onPlaylistReady(MafwPlaylist*,uint,MafwPlayState,const char *,QString)));

    rendererObjectId = objectId;
    ready = true;

    autoSetVisibility();
}
#endif

void NowPlayingIndicator::showEvent(QShowEvent *)
{
#ifdef MAFW
    mafwrenderer->getStatus();
    triggerAnimation();
#endif
}

void NowPlayingIndicator::hideEvent(QHideEvent *)
{
    stopAnimation();
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
    if (inhibited) return;

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
#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist") return;

    if (mafwState == Playing)
        mafwrenderer->pause();
    else if (mafwState == Paused)
        mafwrenderer->resume();
    else if (mafwState == Stopped)
        mafwrenderer->play();
#endif
}
