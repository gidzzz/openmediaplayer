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

#include "nowplayingwindow.h"

NowPlayingWindow* NowPlayingWindow::instance = NULL;

NowPlayingWindow* NowPlayingWindow::acquire(QWidget *parent, MafwRegistryAdapter *mafwRegistry)
{
    if (instance) {
        qDebug() << "Handing out running NPW instance";
        instance->setParent(parent, Qt::Window);
    }
    else {
        qDebug() << "Handing out new NPW instance";
        instance = new NowPlayingWindow(parent, mafwRegistry);
    }

    return instance;
}

void NowPlayingWindow::destroy()
{
    if (instance) {
        instance->deleteLater();
        instance = NULL;
    }
}

NowPlayingWindow::NowPlayingWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry) :
    BaseWindow(parent),
    ui(new Ui::NowPlayingWindow),
    mafwRegistry(mafwRegistry),
    mafwrenderer(mafwRegistry->renderer()),
    playlist(mafwRegistry->playlist()),
    mafwTrackerSource(mafwRegistry->source(MafwRegistryAdapter::Tracker))
{
    ui->setupUi(this);

    QPalette palette;
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
    palette.setColor(QPalette::WindowText, secondaryColor);

    ui->songNumberLabel->setPalette(palette);
    ui->playlistTimeLabel->setPalette(palette);
    ui->albumLabel->setPalette(palette);
    ui->artistHeaderLabel->setPalette(palette);

    defaultWindowTitle = this->windowTitle();

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    albumArtSceneLarge = new QGraphicsScene(ui->coverViewLarge);
    albumArtSceneSmall = new QGraphicsScene(ui->coverViewSmall);
    qmlView = 0;

    enableLyrics = QSettings().value("lyrics/enable").toBool();
    lazySliders = QSettings().value("main/lazySliders").toBool();
    permanentDelete = QSettings().value("main/permanentDelete").toBool();
    reverseTime = QSettings().value("main/reverseTime").toBool();

    ui->songList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->songList->viewport()->setAcceptDrops(true);
    ui->songList->setAutoScrollMargin(70);
    QApplication::setStartDragDistance(20);
    ui->songList->setDragEnabled(false);
    dragInProgress = false;

    clickedItem = NULL;
    clickTimer = new QTimer(this);
    clickTimer->setInterval(QApplication::doubleClickInterval());
    clickTimer->setSingleShot(true);

    keyTimer = new QTimer(this);
    keyTimer->setInterval(5000);
    keyTimer->setSingleShot(true);

    ui->songList->setItemDelegate(new PlayListDelegate(ui->songList));

    this->setButtonIcons();

    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    playlistRequested = false;
    buttonWasDown = false;
    playlistTime = 0;
    currentViewId = 0;
    swipeStart = -1;

    mafwState = Transitioning;

    lastPlayingSong = new GConfItem("/apps/mediaplayer/last_playing_song", this);

    this->connectSignals();

    MetadataWatcher *mw = MissionControl::acquire()->metadataWatcher();
    connect(mw, SIGNAL(metadataChanged(QString,QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
    QMap<QString,QVariant> metadata = mw->metadata();
    onMetadataChanged(MAFW_METADATA_KEY_TITLE, metadata.value(MAFW_METADATA_KEY_TITLE));
    onMetadataChanged(MAFW_METADATA_KEY_ARTIST, metadata.value(MAFW_METADATA_KEY_ARTIST));
    onMetadataChanged(MAFW_METADATA_KEY_ALBUM, metadata.value(MAFW_METADATA_KEY_ALBUM));
    onMetadataChanged(MAFW_METADATA_KEY_DURATION, metadata.value(MAFW_METADATA_KEY_DURATION));
    onMetadataChanged(MAFW_METADATA_KEY_IS_SEEKABLE, metadata.value(MAFW_METADATA_KEY_IS_SEEKABLE));
    onMetadataChanged(MAFW_METADATA_KEY_MIME, metadata.value(MAFW_METADATA_KEY_MIME));
    onMetadataChanged(MAFW_METADATA_KEY_URI, metadata.value(MAFW_METADATA_KEY_URI));
    onMetadataChanged(MAFW_METADATA_KEY_RENDERER_ART_URI, metadata.value(MAFW_METADATA_KEY_RENDERER_ART_URI));

    ui->songList->viewport()->installEventFilter(this);
    ui->currentPositionLabel->installEventFilter(this);

    Rotator::acquire()->addClient(this);

    if (enableLyrics) {
        LyricsManager *lyricsManager = MissionControl::acquire()->lyricsManager();
        connect(lyricsManager, SIGNAL(lyricsFetched(QString)), this, SLOT(setLyricsNormal(QString)));
        connect(lyricsManager, SIGNAL(lyricsInfo(QString)), this, SLOT(setLyricsInfo(QString)));
        lyricsManager->reloadLyrics();
    }

    playlistQM = new PlaylistQueryManager(this, playlist);
    connect(playlistQM, SIGNAL(onGetItems(QString, GHashTable*, guint)), this, SLOT(onGetPlaylistItems(QString, GHashTable*, guint)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), playlistQM, SLOT(setPriority(int)));

    if (mafwrenderer->isRendererReady()) {
        mafwrenderer->getStatus();
        mafwrenderer->getVolume();
    } else {
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getVolume()));
    }
}

NowPlayingWindow::~NowPlayingWindow()
{
    instance = NULL;
    delete ui;
}

bool NowPlayingWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
        this->setWindowTitle(defaultWindowTitle);
    else if (event->type() == QEvent::WindowDeactivate)
        this->setWindowTitle(ui->titleLabel->whatsThis());

    return QMainWindow::event(event);
}

void NowPlayingWindow::detectAlbumImage(QString image)
{
    if (image.isNull() || !QFileInfo(image).exists()) {
        if (currentURI.isNull()) {
            image = defaultAlbumImage;
        } else {
            // Searh for album art in song's directory
            image = currentURI;
            image.remove("file://").replace("%20"," ");
            image.truncate(image.lastIndexOf("/"));

            if (QFileInfo(image + "/cover.jpg").exists()) {
                image.append("/cover.jpg");
            } else if (QFileInfo(image + "/front.jpg").exists()) {
                image.append("/front.jpg");
            } else if (QFileInfo(image + "/folder.jpg").exists()) {
                image.append("/folder.jpg");
            } else {
                image = QString();
            }

            image = image.isNull()
                  ? defaultAlbumImage
                  : MediaArt::setAlbumImage(ui->albumLabel->text(), image);
        }
    }

    setAlbumImage(image);
}

void NowPlayingWindow::setAlbumImage(QString image)
{
    albumArtPath = image;

    qDeleteAll(albumArtSceneLarge->items());
    qDeleteAll(albumArtSceneSmall->items());

    mirror *m;
    QGraphicsPixmapItem *item;
    QPixmap albumArt(albumArtPath);

    ui->coverViewLarge->setScene(albumArtSceneLarge);
    albumArtSceneLarge->setBackgroundBrush(QBrush(Qt::transparent));
    m = new mirror();
    albumArtSceneLarge->addItem(m);
    albumArt = albumArt.scaled(QSize(295, 295), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    item = new QGraphicsPixmapItem(albumArt);
    albumArtSceneLarge->addItem(item);
    m->setItem(item);

    ui->coverViewSmall->setScene(albumArtSceneSmall);
    albumArtSceneSmall->setBackgroundBrush(QBrush(Qt::transparent));
    m = new mirror();
    albumArtSceneSmall->addItem(m);
    albumArt = albumArt.scaled(QSize(80, 80), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    item = new QGraphicsPixmapItem(albumArt);
    albumArtSceneSmall->addItem(item);
    m->setItem(item);

    /*QTransform t;
    t = t.rotate(-10, Qt::YAxis);
    ui->coverViewLarge->setTransform(t);*/
}

void NowPlayingWindow::setTitle(QString title)
{
    ui->titleLabel->setWhatsThis(title);
    ui->titleLabel->setText(QFontMetrics(ui->titleLabel->font()).elidedText(title, Qt::ElideRight, 425));

    ui->titleHeaderLabel->setText(QFontMetrics(ui->titleHeaderLabel->font()).elidedText(title, Qt::ElideRight, 370));

    if (!this->isActiveWindow())
        this->setWindowTitle(title);
}

void NowPlayingWindow::setArtist(QString artist)
{
    ui->artistLabel->setWhatsThis(artist);
    ui->artistLabel->setText(QFontMetrics(ui->artistLabel->font()).elidedText(artist, Qt::ElideRight, 425));

    ui->artistHeaderLabel->setText(QFontMetrics(ui->artistHeaderLabel->font()).elidedText(artist, Qt::ElideRight, 370));
}

void NowPlayingWindow::setAlbum(QString album)
{
    ui->albumLabel->setWhatsThis(album);
    ui->albumLabel->setText(QFontMetrics(ui->albumLabel->font()).elidedText(album, Qt::ElideRight, 425));
}

void NowPlayingWindow::setSongNumber(int currentSong, int numberOfSongs)
{
    ui->songNumberLabel->setText(QString::number(currentSong) + " / " + tr("%n song(s)", "", numberOfSongs));
}

void NowPlayingWindow::updatePlaylistTimeLabel()
{
    ui->playlistTimeLabel->setText(mmss_len(playlistTime) + " " + tr("total"));
}

void NowPlayingWindow::startPositionTimer()
{
    if (!positionTimer->isActive()
    &&  mafwState == Playing
    &&  !this->isHidden()
    &&  !ui->infoWidget->isHidden()
    &&  !Maemo5DeviceEvents::acquire()->isScreenLocked())
    {
        mafwrenderer->getPosition();
        positionTimer->start();
    }
}

void NowPlayingWindow::toggleVolumeSlider()
{
    if (ui->volumeSlider->isHidden()) {
        ui->buttonsWidget->hide();
        ui->volumeSlider->show();

    } else {
        ui->volumeSlider->hide();
        ui->buttonsWidget->show();

        if (volumeTimer->isActive())
            volumeTimer->stop();
    }
}

void NowPlayingWindow::onPropertyChanged(const QDBusMessage &msg)
{
    /*dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer \
                 /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume*/
    if (msg.arguments()[0].toString() == MAFW_PROPERTY_RENDERER_VOLUME) {
        int volumeLevel = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
#ifdef DEBUG
        qDebug() << QString::number(volumeLevel);
#endif
        if (!ui->volumeSlider->isSliderDown())
            ui->volumeSlider->setValue(volumeLevel);
    }
}

void NowPlayingWindow::setButtonIcons()
{
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void NowPlayingWindow::onStateChanged(int state)
{
    this->mafwState = state;

    if (state == Paused) {
        ui->positionSlider->setEnabled(isMediaSeekable);

        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));

        positionTimer->stop();
        mafwrenderer->getPosition();
    }
    else if (state == Playing) {
        ui->positionSlider->setEnabled(isMediaSeekable);

        ui->playButton->setIcon(QIcon(pauseButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));

        startPositionTimer();
    }
    else if (state == Stopped) {
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));

        positionTimer->stop();
    }

    if (state == Transitioning || state == Stopped) {
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(0);
        ui->currentPositionLabel->setText(mmss_pos(0));
    }
}

void NowPlayingWindow::connectSignals()
{
    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Left), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(previous()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Right), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(next()));
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(cycleView()));
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(cycleViewBack()));

    shortcut = new QShortcut(QKeySequence(Qt::Key_S), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(stop()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_E), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), ui->shuffleButton, SLOT(click()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_R), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), ui->repeatButton, SLOT(click()));

    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionAdd_to_playlist, SIGNAL(triggered()), this, SLOT(onAddAllToPlaylist()));
    connect(ui->actionEntertainment_view, SIGNAL(triggered()), this, SLOT(showEntertainmentView()));
    connect(ui->actionCar_view, SIGNAL(triggered()), this, SLOT(showCarView()));

    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));

    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));

    connect(ui->volumeSlider, SIGNAL(sliderPressed()), this, SLOT(onVolumeSliderPressed()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), this, SLOT(onVolumeSliderReleased()));

    connect(ui->coverViewLarge, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onViewContextMenuRequested(QPoint)));
    connect(ui->coverViewSmall, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onViewContextMenuRequested(QPoint)));
    connect(ui->lyricsText,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onLyricsContextMenuRequested(QPoint)));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->repeatButton, SIGNAL(clicked(bool)), this, SLOT(onRepeatButtonToggled(bool)));
    connect(ui->shuffleButton, SIGNAL(clicked(bool)), this, SLOT(onShuffleButtonToggled(bool)));

    connect(ui->nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));
    connect(ui->nextButton, SIGNAL(released()), this, SLOT(onNextButtonPressed()));
    connect(ui->prevButton, SIGNAL(pressed()), this, SLOT(onPrevButtonPressed()));
    connect(ui->prevButton, SIGNAL(released()), this, SLOT(onPrevButtonPressed()));

    connect(ui->positionSlider, SIGNAL(sliderPressed()), this, SLOT(onPositionSliderPressed()));
    connect(ui->positionSlider, SIGNAL(sliderReleased()), this, SLOT(onPositionSliderReleased()));
    connect(ui->positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(onPositionSliderMoved(int)));

    connect(keyTimer, SIGNAL(timeout()), this, SLOT(onKeyTimeout()));
    connect(clickTimer, SIGNAL(timeout()), this, SLOT(forgetClick()));

    connect(ui->songList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked()));
    connect(this, SIGNAL(itemDropped(QListWidgetItem*, int)), this, SLOT(onItemDropped(QListWidgetItem*, int)), Qt::QueuedConnection);

    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));

    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int,char*)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onPositionChanged(int, QString)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));

    connect(ui->playButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPlayMenuRequested(QPoint)));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));

    connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));
    connect(ui->actionClear_now_playing, SIGNAL(triggered()), this, SLOT(clearPlaylist()));
    connect(lastPlayingSong, SIGNAL(valueChanged()), this, SLOT(onGconfValueChanged()));

    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onPropertyChanged(const QDBusMessage &)));

    QDBusConnection::sessionBus().connect("", "", "com.nokia.mafw.playlist", "property_changed",
                                          this, SLOT(updatePlaylistState()));
    connect(playlist, SIGNAL(contentsChanged(guint, guint, guint)), this, SLOT(updatePlaylist(guint, guint, guint)));
    connect(playlist, SIGNAL(itemMoved(guint, guint)), this, SLOT(onItemMoved(guint, guint)));

    // What's this? It's totally undocumented -- the only place where I have
    // found it mentioned is a header file for Harmattan. Moreover, I don't
    // think I have ever seen this signal being emitted.
    QDBusConnection::sessionBus().connect("", "", "com.nokia.mafw.playlist", "playlist_updated", this, SLOT(updatePlaylist()));
}

void NowPlayingWindow::onScreenLocked(bool locked)
{
    if (locked) {
        positionTimer->stop();
    } else {
        startPositionTimer();
    }
}

void NowPlayingWindow::showFMTXDialog()
{
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
}

void NowPlayingWindow::onKeyTimeout()
{
    focusItemByRow(lastPlayingSong->value().toInt());
}

void NowPlayingWindow::forgetClick()
{
    if (clickedItem) onPlaylistItemActivated(clickedItem);
    ui->songList->setDragEnabled(false);
    selectItemByRow(lastPlayingSong->value().toInt());
    clickedItem = NULL;
}

bool NowPlayingWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->currentPositionLabel && event->type() == QEvent::MouseButtonPress) {
        reverseTime = !reverseTime;
        QSettings().setValue("main/reverseTime", reverseTime);
        ui->currentPositionLabel->setText(mmss_pos(reverseTime ? ui->positionSlider->value()-songDuration :
                                                                 ui->positionSlider->value()));
    }

    else if (object == ui->songList->viewport()) {
        if (event->type() == QEvent::DragMove) {
            dragInProgress = true;
        }

        else if (event->type() == QEvent::Drop) {
            static_cast<QDropEvent*>(event)->setDropAction(Qt::MoveAction);
            emit itemDropped(ui->songList->currentItem(), ui->songList->currentRow());
            dragInProgress = false;
        }

        else if (event->type() == QEvent::Leave) {
            dragInProgress = false;
            selectItemByRow(lastPlayingSong->value().toInt());
        }

        else if (event->type() == QEvent::MouseButtonPress) {
            clickedItem = ui->songList->itemAt(0, static_cast<QMouseEvent*>(event)->y());
        }

        else if (event->type() == QEvent::MouseButtonRelease) {
            if (clickedItem != ui->songList->itemAt(0, static_cast<QMouseEvent*>(event)->y()))
                clickedItem = NULL;
            clickTimer->start();
        }
    }

    return false;
}

void NowPlayingWindow::onItemDoubleClicked()
{
    ui->songList->setDragEnabled(true);
    clickedItem = NULL;
    clickTimer->start();
}

void NowPlayingWindow::onItemDropped(QListWidgetItem *item, int from)
{
    playlist->moveItem(from, ui->songList->row(item));
}

void NowPlayingWindow::onItemMoved(guint from, guint to)
{
    playlistQM->itemsRemoved(from, 1);
    playlistQM->itemsInserted(to, 1);

    if (ui->songList->item(to)->data(UserRoleSongDuration) == Duration::Blank)
        playlistQM->getItems(to,to);

    mafwrenderer->getStatus();
}

void NowPlayingWindow::onMetadataChanged(QString key, QVariant value)
{
    if (key == MAFW_METADATA_KEY_TITLE) {
        if (value.isNull()) {
            setTitle(tr("(unknown song)"));
        } else {
            QString title = value.toString();
            setTitle(title);

            QListWidgetItem* item = ui->songList->item(lastPlayingSong->value().toInt());
            if (item && item->data(UserRoleSongTitle).toString().isEmpty())
                item->setData(UserRoleSongTitle, title);
        }
    }
    else if (key == MAFW_METADATA_KEY_ARTIST) {
        if (value.isNull()) {
            setArtist(tr("(unknown artist)"));
        } else {
            QString artist = value.toString();
            setArtist(artist);

            QListWidgetItem* item = ui->songList->item(lastPlayingSong->value().toInt());
            if (item && item->data(UserRoleSongArtist).toString().isEmpty())
                item->setData(UserRoleSongArtist, artist);
        }
    }
    else if (key == MAFW_METADATA_KEY_ALBUM) {
        if (value.isNull()) {
            setAlbum(tr("(unknown album)"));
        } else {
            QString album = value.toString();
            setAlbum(album);

            QListWidgetItem* item = ui->songList->item(lastPlayingSong->value().toInt());
            if (item && item->data(UserRoleSongAlbum).toString().isEmpty())
                item->setData(UserRoleSongAlbum, album);
        }
    }
    else if (key == MAFW_METADATA_KEY_DURATION) {
        if (value.isNull()) {
            songDuration = Duration::Unknown;
        } else {
            songDuration = value.toInt();

            if (songDuration < 0)
                songDuration = Duration::Unknown;

            QListWidgetItem* item = ui->songList->item(lastPlayingSong->value().toInt());
            if (item && item->data(UserRoleSongDuration).toInt() == Duration::Blank) {
                item->setData(UserRoleSongDuration, songDuration);
                if (songDuration > 0) {
                    playlistTime += songDuration;
                    updatePlaylistTimeLabel();
                }
            }
        }
        ui->trackLengthLabel->setText(mmss_len(songDuration));
        ui->positionSlider->setRange(0, songDuration);
    }
    else if (key == MAFW_METADATA_KEY_IS_SEEKABLE) {
        isMediaSeekable = value.toBool();

        if (mafwState == Playing || mafwState == Paused)
            ui->positionSlider->setEnabled(isMediaSeekable);
    }
    else if (key == MAFW_METADATA_KEY_URI) {
        currentURI = value.toString();

        if (albumArtPath == defaultAlbumImage)
            detectAlbumImage();
    }
    else if (key == MAFW_METADATA_KEY_MIME) {
        currentMIME = value.isNull() ? "audio/unknown" : value.toString();
    }
    else if (key == MAFW_METADATA_KEY_RENDERER_ART_URI) {
        detectAlbumImage(value.toString());
    }

    updateQmlViewMetadata();
}

void NowPlayingWindow::setLyricsNormal(QString lyrics)
{
    setLyrics(lyrics, QMaemo5Style::standardColor("DefaultTextColor"));
}

void NowPlayingWindow::setLyricsInfo(QString info)
{
    setLyrics(info, QMaemo5Style::standardColor("SecondaryTextColor"));
}

void NowPlayingWindow::setLyrics(QString text, QColor color)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    ui->lyricsText->setPalette(palette);

    ui->lyricsText->setText(text);
    ui->lyricsText->adjustSize();
    ui->lyricsArea->verticalScrollBar()->setValue(0);
}

void NowPlayingWindow::editLyrics()
{
    (new LyricsEditDialog(ui->artistLabel->whatsThis(), ui->titleLabel->whatsThis(), this))->show();
}

void NowPlayingWindow::searchLyrics()
{
    (new LyricsSearchDialog(this))->show();
}

void NowPlayingWindow::onOrientationChanged(int w, int h)
{
    if (w > h) { // Landscape mode
        portrait = false;
        ui->orientationLayout->setDirection(QBoxLayout::LeftToRight);
        if (ui->volumeWidget->isHidden())
            ui->volumeWidget->show();

        ui->infoLayout->setContentsMargins(0,0,16,0);
        ui->volumeWidget->setContentsMargins(0,0,9,9);
        ui->buttonsLayout->setSpacing(60);

        ui->titleLabel->setAlignment(Qt::AlignLeft);
        ui->artistLabel->setAlignment(Qt::AlignLeft);
        ui->albumLabel->setAlignment(Qt::AlignLeft);
        ui->lyricsText->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

        ui->headerWidget->hide();
        ui->coverViewLarge->show();

    } else { // Portrait mode
        portrait = true;
        ui->orientationLayout->setDirection(QBoxLayout::TopToBottom);
        if (!ui->volumeButton->isHidden())
            ui->volumeWidget->hide();

        ui->infoLayout->setContentsMargins(16,0,16,0);
        ui->volumeWidget->setContentsMargins(18,0,9,9);
        ui->buttonsLayout->setSpacing(27);

        ui->titleLabel->setAlignment(Qt::AlignHCenter);
        ui->artistLabel->setAlignment(Qt::AlignHCenter);
        ui->albumLabel->setAlignment(Qt::AlignHCenter);
        ui->lyricsText->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        if (ui->infoWidget->isHidden()) {
            ui->coverViewLarge->hide();
            ui->headerWidget->show();
        }
    }
}

void NowPlayingWindow::cycleView(int direction)
{
    const int numViews = 2 + enableLyrics;
    currentViewId = (currentViewId + direction + numViews) % numViews;

    switch (currentViewId) {
        case 0: // Song info view
            if (portrait) {
                ui->headerWidget->hide();
                ui->coverViewLarge->show();
            }
            ui->lyricsArea->hide();
            ui->songList->hide();
            ui->infoWidget->show();
            startPositionTimer();
            break;

        case 1: // Playlist view
            if (portrait) {
                ui->coverViewLarge->hide();
                ui->headerWidget->show();
            }
            ui->infoWidget->hide();
            ui->lyricsArea->hide();
            ui->songList->show();
            ui->songList->setFocus();
            positionTimer->stop();
            break;

        case 2: // Lyrics view
            if (portrait) {
                ui->coverViewLarge->hide();
                ui->headerWidget->show();
            }
            ui->infoWidget->hide();
            ui->songList->hide();
            ui->lyricsArea->show();
            ui->lyricsArea->setFocus();
            positionTimer->stop();
            break;
    }
}

void NowPlayingWindow::cycleViewBack()
{
    cycleView(-1);
}

void NowPlayingWindow::volumeWatcher()
{
    if (!ui->volumeSlider->isHidden())
        volumeTimer->start();
}

void NowPlayingWindow::onShuffleButtonToggled(bool checked)
{
    if (checked) {
        ui->shuffleButton->setIcon(QIcon(shuffleButtonPressed));
    } else {
        ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    }

    playlist->setShuffled(checked);
}

void NowPlayingWindow::onRepeatButtonToggled(bool checked)
{
    if (checked) {
        ui->repeatButton->setIcon(QIcon(repeatButtonPressedIcon));
    } else {
        ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    }

    playlist->setRepeat(checked);
}

void NowPlayingWindow::onNextButtonPressed()
{
    if (ui->nextButton->isDown()) {
        ui->nextButton->setIcon(QIcon(nextButtonPressedIcon));
    } else {
        ui->nextButton->setIcon(QIcon(nextButtonIcon));
    }
}

void NowPlayingWindow::onPrevButtonPressed()
{
    if (ui->prevButton->isDown())
        ui->prevButton->setIcon(QIcon(prevButtonPressedIcon));
    else
        ui->prevButton->setIcon(QIcon(prevButtonIcon));
}

void NowPlayingWindow::onVolumeSliderPressed()
{
    volumeTimer->stop();
    mafwrenderer->setVolume(ui->volumeSlider->value());
}

void NowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
    mafwrenderer->setVolume(ui->volumeSlider->value());
}

void NowPlayingWindow::onPositionSliderPressed()
{
    onPositionSliderMoved(ui->positionSlider->value());
}

void NowPlayingWindow::onPositionSliderReleased()
{
    mafwrenderer->setPosition(SeekAbsolute, ui->positionSlider->value());
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? ui->positionSlider->value()-songDuration :
                                                             ui->positionSlider->value()));
}

void NowPlayingWindow::onPositionSliderMoved(int position)
{
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? position-songDuration : position));
    if (!lazySliders)
        mafwrenderer->setPosition(SeekAbsolute, position);
}

void NowPlayingWindow::onPlayMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Stop playback"), mafwrenderer, SLOT(stop()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void NowPlayingWindow::onNextButtonClicked()
{
    if (ui->nextButton->isDown()) {
        buttonWasDown = true;
        if (currentSongPosition >= this->songDuration)
            mafwrenderer->setPosition(SeekAbsolute, 0);
        else
            mafwrenderer->setPosition(SeekRelative, 3);
        mafwrenderer->getPosition();
    } else {
        if (!buttonWasDown)
            mafwrenderer->next();
        buttonWasDown = false;
    }
}

void NowPlayingWindow::onPreviousButtonClicked()
{
    if (ui->prevButton->isDown()) {
        buttonWasDown = true;
        mafwrenderer->setPosition(SeekRelative, -3);
        mafwrenderer->getPosition();
    } else {
        if (!buttonWasDown) {
            if (this->currentSongPosition > 3)
                this->setPosition(0);
            else
                mafwrenderer->previous();
        }
        buttonWasDown = false;
    }
}

void NowPlayingWindow::onPositionChanged(int position, QString)
{
    currentSongPosition = position;
    if (!ui->positionSlider->isSliderDown())
        ui->currentPositionLabel->setText(mmss_pos(reverseTime ? position-songDuration : position));

    if (this->songDuration != 0 && this->songDuration != -1 && qmlView == 0) {
#ifdef DEBUG
        qDebug() << "Current position: " << position;
        qDebug() << "Song Length: " << this->songDuration;
#endif
        if (!ui->positionSlider->isSliderDown() && ui->positionSlider->isVisible())
            ui->positionSlider->setValue(position);
    }
}

void NowPlayingWindow::onGetStatus(MafwPlaylist*, uint index, MafwPlayState state, const char*, QString)
{
    if (!playlistRequested) {
        updatePlaylist();
        updatePlaylistState();
        onStateChanged(state);
        playlistRequested = true;
    }

    lastPlayingSong->set((int)index); // sometimes the value is wrong, noticable mainly while using the playlist editor
    setSongNumber(index+1, playlist->getSize());
    selectItemByRow(index);
}

void NowPlayingWindow::setPosition(int newPosition)
{
    mafwrenderer->setPosition(SeekAbsolute, newPosition);
    mafwrenderer->getPosition();
}

void NowPlayingWindow::showEvent(QShowEvent *)
{
    this->setWindowTitle(defaultWindowTitle); // avoid showing a different title for a split second

    startPositionTimer();
}

void NowPlayingWindow::onGconfValueChanged()
{
    setSongNumber(lastPlayingSong->value().toInt()+1, ui->songList->count());
    selectItemByRow(lastPlayingSong->value().toInt());
}

void NowPlayingWindow::onMediaChanged(int index, char*)
{
    lastPlayingSong->set(index);
    focusItemByRow(index);
}

void NowPlayingWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_Enter:
            if (e->modifiers() & Qt::ControlModifier) {
                if (ui->songList->isVisible())
                    onContextMenuRequested(QPoint(this->width()/2,35));
                else if (ui->lyricsArea->isVisible())
                    onLyricsContextMenuRequested(QPoint(this->width()/2,35));
                else
                    onViewContextMenuRequested(QPoint(35,35));
            } else {
                onPlaylistItemActivated(ui->songList->currentItem());
            }
            break;
    }
}

void NowPlayingWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
        keyTimer->start();
}

void NowPlayingWindow::mousePressEvent(QMouseEvent *e)
{
    if (!ui->headerWidget->isHidden() && ui->headerWidget->frameGeometry().contains(e->pos())
    ||  !ui->coverViewLarge->isHidden() && ui->orientationLayout->itemAt(0)->geometry().contains(e->pos()))
        swipeStart = e->x();
}

void NowPlayingWindow::mouseReleaseEvent(QMouseEvent *e)
{
    // Accept only gestures on cover area's level
    if (swipeStart == -1
    || !ui->headerWidget->isHidden() && e->y() > ui->headerWidget->frameGeometry().bottom()
    || !ui->coverViewLarge->isHidden() && e->y() > ui->orientationLayout->itemAt(0)->geometry().bottom())
        return;

    if (e->x() - swipeStart > 100) {
        cycleViewBack();
    } else {
        cycleView();
    }

    swipeStart = -1;
}

void NowPlayingWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    mousePressEvent(e);
}

void NowPlayingWindow::togglePlayback()
{
    if (mafwState == Playing)
        mafwrenderer->pause();
    else if (mafwState == Paused)
        mafwrenderer->resume();
    else if (mafwState == Stopped)
        mafwrenderer->play();
}

void NowPlayingWindow::onGetPlaylistItems(QString objectId, GHashTable *metadata, guint index)
{
    QListWidgetItem *item = ui->songList->item(index);

    if (!item) return; // in case of query manager's outdated request

    if (item->data(UserRoleSongDuration).toInt() > 0)
        playlistTime -= item->data(UserRoleSongDuration).toInt();

    if (metadata != NULL) {
        QString title;
        QString artist;
        QString album;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        if (duration > 0)
            playlistTime += duration;

        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, objectId);
    } else {
        item->setData(UserRoleSongTitle, tr("Information not available"));
        item->setData(UserRoleSongDuration, Duration::Blank);
    }

    updatePlaylistTimeLabel();

    if (qmlView) qmlView->setPlaylistItem(index, item);
}

void NowPlayingWindow::onPlaylistItemActivated(QListWidgetItem *item)
{
#ifdef DEBUG
    qDebug() << "Selected item number: " << ui->songList->currentRow();
#endif
    setSongNumber(ui->songList->currentRow()+1, ui->songList->count());
    lastPlayingSong->set(ui->songList->currentRow());

    setTitle(item->data(UserRoleSongTitle).toString());
    setArtist(item->data(UserRoleSongArtist).toString());
    setAlbum(item->data(UserRoleSongAlbum).toString());

    ui->currentPositionLabel->setText(mmss_pos(0));
    songDuration = item->data(UserRoleSongDuration).toInt();
    ui->trackLengthLabel->setText(mmss_len(songDuration));

    mafwrenderer->gotoIndex(ui->songList->row(item));
    if (mafwState == Stopped || mafwState == Paused)
        mafwrenderer->play();
}

void NowPlayingWindow::updatePlaylistState()
{
    if (playlist->isShuffled()) {
        ui->shuffleButton->setIcon(QIcon(shuffleButtonPressed));
        ui->shuffleButton->setChecked(true);
    } else {
        ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
        ui->shuffleButton->setChecked(false);
    }

    if (playlist->isRepeat()) {
        ui->repeatButton->setIcon(QIcon(repeatButtonPressedIcon));
        ui->repeatButton->setChecked(true);
    } else {
        ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
        ui->repeatButton->setChecked(false);
    }
}

void NowPlayingWindow::clearPlaylist()
{
    if (ConfirmDialog(ConfirmDialog::ClearCurrent, this).exec() == QMessageBox::Yes) {
        playlist->clear();
        lastPlayingSong->set(1);
        this->close();
    }
}

void NowPlayingWindow::onContextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Delete from now playing"), this, SLOT(onDeleteFromNowPlaying()));
    if (!ui->songList->currentItem()->data(UserRoleObjectID).toString().startsWith("_uuid_")) {
        if (permanentDelete) contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
        contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
        contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(onRingtoneClicked()));
        contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    }
    contextMenu->exec(this->mapToGlobal(pos));
}

void NowPlayingWindow::onAddToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
        playlist->appendItem(picker.playlist, ui->songList->currentItem()->data(UserRoleObjectID).toString());
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
    }
}

void NowPlayingWindow::onRingtoneClicked()
{
    (new RingtoneDialog(this, mafwTrackerSource,
                        ui->songList->currentItem()->data(UserRoleObjectID).toString(),
                        ui->songList->currentItem()->data(UserRoleSongTitle).toString(),
                        ui->songList->currentItem()->data(UserRoleSongArtist).toString()))
    ->show();
}

void NowPlayingWindow::onDeleteClicked()
{
    if (ConfirmDialog(ConfirmDialog::DeleteSong, this,
                      ui->songList->currentItem()->data(UserRoleSongTitle).toString(),
                      ui->songList->currentItem()->data(UserRoleSongArtist).toString())
        .exec() == QMessageBox::Yes)
    {
        playlist->removeItem(ui->songList->currentRow());
        mafwTrackerSource->destroyObject(ui->songList->currentItem()->data(UserRoleObjectID).toString());
    }
}

void NowPlayingWindow::onShareClicked()
{
    (new ShareDialog(this, mafwTrackerSource, ui->songList->currentItem()->data(UserRoleObjectID).toString()))->show();
}

void NowPlayingWindow::showEntertainmentView()
{
    createQmlView(QUrl("file:///opt/openmediaplayer/qml/entertainmentview/entertainmentview.qml"), tr("Entertainment View"));
}

void NowPlayingWindow::showCarView()
{
    createQmlView(QUrl("file:///opt/openmediaplayer/qml/carview/carview.qml"), tr("Car View"));
}

void NowPlayingWindow::createQmlView(QUrl source, QString title)
{
    if (!qmlView) {
        qmlView = new QmlView(source, this, mafwRegistry);
        qmlView->setWindowTitle(title);
        for (int i = 0; i < ui->songList->count(); i++)
            qmlView->appendPlaylistItem(ui->songList->item(i));
        connect(qmlView, SIGNAL(destroyed()), this, SLOT(nullQmlView()));
        updateQmlViewMetadata();
    }
    qmlView->showFullScreen();
}

void NowPlayingWindow::updateQmlViewMetadata()
{
    if (qmlView) {
        qmlView->setMetadata(ui->titleLabel->text(),
                             ui->albumLabel->text(),
                             ui->artistLabel->text(),
                             this->albumArtPath,
                             this->songDuration);
        qmlView->setCurrentRow(ui->songList->currentRow());
    }
}

void NowPlayingWindow::nullQmlView()
{
    qmlView = 0;
    mafwrenderer->getPosition();
}

void NowPlayingWindow::onAddAllToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
        int songCount = ui->songList->count();
        gchar** songAddBuffer = new gchar*[songCount+1];

        for (int i = 0; i < songCount; i++)
            songAddBuffer[i] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());

        songAddBuffer[songCount] = NULL;

        playlist->appendItems(picker.playlist, (const gchar**) songAddBuffer);

        for (int i = 0; i < songCount; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", songCount));
    }
}

void NowPlayingWindow::onDeleteFromNowPlaying()
{
    playlist->removeItem(ui->songList->currentRow());
}

void NowPlayingWindow::selectItemByRow(int row)
{
    if (!dragInProgress && ui->songList->item(row)) {
        ui->songList->clearSelection();
        ui->songList->item(row)->setSelected(true);
    }
}

void NowPlayingWindow::focusItemByRow(int row)
{
    if (!dragInProgress && ui->songList->item(row)) {
        // Prevent instant scrolling caused by setCurrentRow()
        int pos = ui->songList->verticalScrollBar()->value();
        ui->songList->setCurrentRow(row);
        ui->songList->verticalScrollBar()->setValue(pos);

        // Scroll smoothly
        ui->songList->property("kineticScroller").value<QAbstractKineticScroller*>()
                    ->scrollTo(QPoint(0, qBound(0, row*70 + 35-ui->songList->height()/2, ui->songList->verticalScrollBar()->maximum())));
    }
}

void NowPlayingWindow::updatePlaylist(guint from, guint nremove, guint nreplace)
{
    qDebug() << "Playlist contents changed: @" << from << "-" << nremove << "+" << nreplace;

    if (playlist->playlistName() != "FmpAudioPlaylist") {
        qDebug() << "Playlist type rejected, update aborted";
        return;
    }

    bool synthetic = from == (uint) -1;

    if (synthetic) {
        playlistTime = 0;
        ui->songList->clear();
        if (qmlView) qmlView->clearPlaylist();

        from = 0;
        nreplace = playlist->getSize();
    }

    if (nremove > 0) {
        for (uint i = 0; i < nremove; i++) {
            QListWidgetItem *item = ui->songList->takeItem(from);
            if (item && item->data(UserRoleSongDuration).toInt() > 0) {
                playlistTime -= item->data(UserRoleSongDuration).toInt();
                delete item;
            }

            if (qmlView) qmlView->removePlaylistItem(from);
        }
        updatePlaylistTimeLabel();
        playlistQM->itemsRemoved(from, nremove);
    }
    else if (nreplace > 0) {
        gchar** items = mafw_playlist_get_items(playlist->mafw_playlist, from, from+nreplace-1, NULL);
        for (int i = 0; items[i] != NULL; i++) {
            QListWidgetItem *item = new QListWidgetItem();
            item->setData(UserRoleSongDuration, Duration::Blank);
            item->setData(UserRoleObjectID, QString::fromUtf8(items[i]));
            ui->songList->insertItem(from+i, item);

            if (qmlView) qmlView->insertPlaylistItem(from+i, item);
        }
        g_strfreev(items);

        if (!synthetic) playlistQM->itemsInserted(from, nreplace);
        playlistQM->getItems(from, from+nreplace-1);
    }

    if (synthetic)
        focusItemByRow(lastPlayingSong->value().toInt());

    mafwrenderer->getStatus();

    setSongNumber(lastPlayingSong->value().toInt()+1, ui->songList->count());

    qDebug() << "Playlist reserved slots:" << ui->songList->count();
}

void NowPlayingWindow::onViewContextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Select album art"), this, SLOT(selectAlbumArt()));
    contextMenu->addAction(tr("Reset album art"), this, SLOT(resetAlbumArt()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void NowPlayingWindow::selectAlbumArt()
{
    CoverPicker picker(ui->albumLabel->whatsThis(), "/home/user/MyDocs", this);
    if (picker.exec() == QDialog::Accepted)
        detectAlbumImage(MediaArt::setAlbumImage(ui->albumLabel->whatsThis(), picker.cover));
}

void NowPlayingWindow::resetAlbumArt()
{
    if (ConfirmDialog(ConfirmDialog::ResetArt, this).exec() == QMessageBox::Yes) {
        // Remove old art and search again
        MediaArt::setAlbumImage(ui->albumLabel->text(), QString());
        detectAlbumImage();

        // Even if detectAlbumImage() falls back to the default art, there still
        // might be an embedded image, so poke Tracker to recheck the song file.
        if (albumArtPath == defaultAlbumImage) {
            QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Tracker.Extract",
                                                              "/org/freedesktop/Tracker/Extract",
                                                              "", "GetMetadata");
            QList<QVariant> args;
            QString file = currentURI;
            file.remove("file://");
            args.append(file);
            args.append(currentMIME);
            msg.setArguments(args);
            QDBusConnection::sessionBus().send(msg);

            // Give Tracker some time to do its job
            QTimer::singleShot(3000, this, SLOT(refreshAlbumArt()));
        }
    }
}

void NowPlayingWindow::refreshAlbumArt()
{
    QString image = MediaArt::albumArtPath(ui->albumLabel->text());
    if (QFileInfo(image).exists())
        setAlbumImage(image);
}

void NowPlayingWindow::onLyricsContextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Edit lyrics"), this, SLOT(editLyrics()));
    contextMenu->addAction(tr("Search for lyrics"), this, SLOT(searchLyrics()));
    contextMenu->addAction(tr("Reload lyrics"), MissionControl::acquire()->lyricsManager(), SLOT(reloadLyricsOverridingCache()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void NowPlayingWindow::closeEvent(QCloseEvent *)
{
    this->setParent(0);

    positionTimer->stop();

    emit hidden();
}
