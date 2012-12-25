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

NowPlayingWindow* NowPlayingWindow::acquire(QWidget *parent, MafwAdapterFactory *mafwFactory)
{
    if (instance) {
        instance->setParent(parent, Qt::Window);
        qDebug() << "Handing out running NPW instance";
    }
    else {
        qDebug() << "Handing out new NPW instance";
        instance = new NowPlayingWindow(parent, mafwFactory);
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

NowPlayingWindow::NowPlayingWindow(QWidget *parent, MafwAdapterFactory *factory) :
    BaseWindow(parent),
#ifdef MAFW
    ui(new Ui::NowPlayingWindow),
    mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    metadataSource(factory->getTempSource()),
    playlist(factory->getPlaylistAdapter())
#else
    ui(new Ui::NowPlayingWindow)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif
    ui->songNumberLabel->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                              .arg(secondaryColor.red())
                              .arg(secondaryColor.green())
                              .arg(secondaryColor.blue()));
    ui->playlistTimeLabel->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                              .arg(secondaryColor.red())
                              .arg(secondaryColor.green())
                              .arg(secondaryColor.blue()));
    ui->albumLabel->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                              .arg(secondaryColor.red())
                              .arg(secondaryColor.green())
                              .arg(secondaryColor.blue()));
    defaultWindowTitle = this->windowTitle();

    setAttribute(Qt::WA_DeleteOnClose);

    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    albumArtSceneLarge = new QGraphicsScene(ui->view_large);
    albumArtSceneSmall = new QGraphicsScene(ui->view_small);
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

    isMediaSeekable = false;
    playlistRequested = false;
    buttonWasDown = false;
    playlistTime = 0;

#ifdef Q_WS_MAEMO_5
    lastPlayingSong = new GConfItem("/apps/mediaplayer/last_playing_song", this);
#endif

    this->onStateChanged(mafwFactory->mafwState());

    this->connectSignals();

    ui->songList->viewport()->installEventFilter(this);
    ui->currentPositionLabel->installEventFilter(this);

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());

    if (enableLyrics) {
        lyricsManager = new LyricsManager(this);
        connect(lyricsManager, SIGNAL(lyricsFetched(QString)), this, SLOT(setLyrics(QString)));
        connect(lyricsManager, SIGNAL(lyricsError(QString)), this, SLOT(setLyrics(QString)));
    }

#ifdef MAFW
    playlistQM = new PlaylistQueryManager(this, playlist);
    connect(playlistQM, SIGNAL(onGetItems(QString, GHashTable*, guint)), this, SLOT(onGetPlaylistItems(QString, GHashTable*, guint)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), playlistQM, SLOT(setPriority(int)));

    if (mafwrenderer->isRendererReady()) {
        mafwrenderer->getCurrentMetadata();
        mafwrenderer->getStatus();
        mafwrenderer->getVolume();
    } else {
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getCurrentMetadata()));
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getVolume()));
    }
#endif
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

void NowPlayingWindow::setAlbumImage(QString image)
{
    qDeleteAll(albumArtSceneLarge->items());
    qDeleteAll(albumArtSceneSmall->items());

    if (image == defaultAlbumImage) {
        // If there's no album art, search for it in song's directory
        QString dirArtPath = currentURI;
        dirArtPath.remove("file://").replace("%20"," ");
        dirArtPath.truncate(dirArtPath.lastIndexOf("/"));

        if (QFileInfo(dirArtPath + "/cover.jpg").exists())
            dirArtPath.append("/cover.jpg");
        else if (QFileInfo(dirArtPath + "/front.jpg").exists())
            dirArtPath.append("/front.jpg");
        else
            dirArtPath.append("/folder.jpg");

        albumArtPath = QFileInfo(dirArtPath).exists() ? MediaArt::setAlbumImage(ui->albumLabel->text(), dirArtPath) : defaultAlbumImage;
    } else {
        albumArtPath = QFileInfo(image).exists() ? image : defaultAlbumImage;
    }

    mirror *m;
    QGraphicsPixmapItem *item;
    QPixmap albumArt(albumArtPath);

    ui->view_large->setScene(albumArtSceneLarge);
    albumArtSceneLarge->setBackgroundBrush(QBrush(Qt::transparent));
    m = new mirror();
    albumArtSceneLarge->addItem(m);
    albumArt = albumArt.scaled(QSize(295, 295), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    item = new QGraphicsPixmapItem(albumArt);
    albumArtSceneLarge->addItem(item);
    m->setItem(item);

    ui->view_small->setScene(albumArtSceneSmall);
    albumArtSceneSmall->setBackgroundBrush(QBrush(Qt::transparent));
    m = new mirror();
    albumArtSceneSmall->addItem(m);
    albumArt = albumArt.scaled(QSize(245, 245), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    item = new QGraphicsPixmapItem(albumArt);
    albumArtSceneSmall->addItem(item);
    m->setItem(item);

    /*QTransform t;
    t = t.rotate(-10, Qt::YAxis);
    ui->view_large->setTransform(t);*/
}

void NowPlayingWindow::setSongNumber(int currentSong, int numberOfSongs)
{
    ui->songNumberLabel->setText(QString::number(currentSong) + " / " + tr("%n song(s)", "", numberOfSongs));
}

void NowPlayingWindow::updatePlaylistTimeLabel()
{
    ui->playlistTimeLabel->setText(mmss_len(playlistTime) + " " + tr("total"));
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

#ifdef MAFW
void NowPlayingWindow::onPropertyChanged(const QDBusMessage &msg)
{
    /*dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer \
                 /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume*/
    if (msg.arguments()[0].toString() == "volume") {
        int volumeLevel = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
#ifdef DEBUG
        qDebug() << QString::number(volumeLevel);
#endif
        if (!ui->volumeSlider->isSliderDown())
            ui->volumeSlider->setValue(volumeLevel);
    }
}
#endif

void NowPlayingWindow::setButtonIcons()
{
    setAlbumImage(defaultAlbumImage);
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void NowPlayingWindow::onMetadataChanged(QString name, QVariant value)
{
    QFont f = ui->titleLabel->font();
    QFontMetrics fm(f);
    QString elided = fm.elidedText(value.toString(), Qt::ElideRight, 425);

    if (name == MAFW_METADATA_KEY_TITLE) {
        ui->titleLabel->setText(elided);
        ui->titleLabel->setWhatsThis(value.toString());
        if (!this->isActiveWindow())
            this->setWindowTitle(value.toString());
    }

    else if (name == MAFW_METADATA_KEY_ARTIST) {
        ui->artistLabel->setText(elided);
        ui->artistLabel->setWhatsThis(value.toString());
    }

    else if (name == MAFW_METADATA_KEY_ALBUM) {
        ui->albumLabel->setWhatsThis(value.toString());
        ui->albumLabel->setText(elided);
    }

    else if (name == MAFW_METADATA_KEY_DURATION) {
        songDuration = value.toInt();
        ui->trackLengthLabel->setText(mmss_len(songDuration));
        ui->positionSlider->setRange(0, songDuration);
    }

    else if (name == MAFW_METADATA_KEY_URI) {
        currentURI = value.toString();
    }

    // else if (name == "MAFW_METADATA_KEY_MIME");

    // else if (name == MAFW_METADATA_KEY_ALBUM_ART_URI);

    else if (name == MAFW_METADATA_KEY_RENDERER_ART_URI) {
        setAlbumImage(value.toString());
    }

    // else if (name == "lyrics");

    updateQmlViewMetadata();
}

#ifdef MAFW
void NowPlayingWindow::onStateChanged(int state)
{
    this->mafwState = state;

    if (state == Paused) {
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));
        mafwrenderer->getPosition();
        if (positionTimer->isActive())
            positionTimer->stop();
    }
    else if (state == Playing) {
        ui->positionSlider->setEnabled(isMediaSeekable);
        ui->playButton->setIcon(QIcon(pauseButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
        mafwrenderer->getPosition();
        if (!positionTimer->isActive())
            positionTimer->start();
    }
    else if (state == Stopped) {
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
        if (positionTimer->isActive())
            positionTimer->stop();
    }

    if (state == Transitioning || state == Stopped) {
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(0);
        ui->currentPositionLabel->setText(mmss_pos(0));
    }
}
#endif

void NowPlayingWindow::connectSignals()
{
    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(togglePlayback()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Left), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(previous()));
    shortcut = new QShortcut(QKeySequence(Qt::Key_Right), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), mafwrenderer, SLOT(next()));
    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space), this); shortcut->setAutoRepeat(false);
    connect(shortcut, SIGNAL(activated()), this, SLOT(toggleList()));

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

    connect(ui->view_large, SIGNAL(clicked()), this, SLOT(toggleList()));
    connect(ui->view_small, SIGNAL(clicked()), this, SLOT(toggleList()));

    connect(ui->view_large, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onViewContextMenuRequested(QPoint)));
    connect(ui->view_small, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onViewContextMenuRequested(QPoint)));
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

#ifdef Q_WS_MAEMO_5
    connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));
#endif

#ifdef MAFW
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString, QVariant)), this, SLOT(onMetadataChanged(QString, QVariant)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onPositionChanged(int, QString)));
    connect(mafwrenderer, SIGNAL(mediaIsSeekable(bool)), this, SLOT(onMediaIsSeekable(bool)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetCurrentMetadata(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataRequested(GHashTable*,QString,QString)));

    connect(metadataSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataRequested(QString,GHashTable*,QString)));

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
    // found it mentioned is a header filer for Harmattan. Moreover, I don't
    // think I have ever seen this signal being emitted.
    QDBusConnection::sessionBus().connect("", "", "com.nokia.mafw.playlist", "playlist_updated", this, SLOT(updatePlaylist()));
#endif
}

#ifdef Q_WS_MAEMO_5
void NowPlayingWindow::onScreenLocked(bool locked)
{
    if (locked) {
        if (positionTimer->isActive())
            positionTimer->stop();
    } else {
        if (!positionTimer->isActive() && this->mafwState == Playing)
            positionTimer->start();
        mafwrenderer->getPosition();
    }
}
#endif

void NowPlayingWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
#endif
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
            if (clickedItem != ui->songList->currentItem())
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
#ifdef MAFW
    playlist->moveItem(from, ui->songList->row(item));
#endif
}

#ifdef MAFW
void NowPlayingWindow::onItemMoved(guint from, guint to)
{
    playlistQM->itemsRemoved(from, 1);
    playlistQM->itemsInserted(to, 1);

    if (ui->songList->item(to)->data(UserRoleSongDuration) == Duration::Blank)
        playlistQM->getItems(to,to);

    mafwrenderer->getStatus();
}
#endif

#ifdef MAFW
void NowPlayingWindow::onRendererMetadataRequested(GHashTable* metadata, QString objectId, QString error)
{
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int, char*)), Qt::UniqueConnection);

    QString title;
    QString artist;
    QString album;
    GValue *v;

    const char **keys = new const char *[9];
    keys[0] = MAFW_METADATA_KEY_URI;
    keys[1] = MAFW_METADATA_KEY_MIME;
    keys[2] = MAFW_METADATA_KEY_ALBUM_ART_URI;
    keys[3] = MAFW_METADATA_KEY_RENDERER_ART_URI;
    char currentKey = 4;

    QListWidgetItem* item = ui->songList->item(lastPlayingSong->value().toInt());

    if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE) )) {
        title = QString::fromUtf8(g_value_get_string (v));
        if (item && item->data(UserRoleSongTitle).toString().isEmpty())
            item->setData(UserRoleSongTitle, title);
    } else {
        title = tr("(unknown song)");
        keys[currentKey++] = MAFW_METADATA_KEY_TITLE;
    }

    if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST) )) {
        artist = QString::fromUtf8(g_value_get_string(v));
        if (item && item->data(UserRoleSongArtist).toString().isEmpty())
            item->setData(UserRoleSongArtist, artist);
    } else {
        artist = tr("(unknown artist)");
        keys[currentKey++] = MAFW_METADATA_KEY_ARTIST;
    }

    if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM) )) {
        album = QString::fromUtf8(g_value_get_string(v));
        if (item && item->data(UserRoleSongAlbum).toString().isEmpty())
            item->setData(UserRoleSongAlbum, album);
    } else {
        album = tr("(unknown album)");
        keys[currentKey++] = MAFW_METADATA_KEY_ALBUM;
    }

    if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION) )) {
        songDuration = g_value_get_int64 (v);
        if (item && item->data(UserRoleSongDuration).toInt() == Duration::Blank)
            item->setData(UserRoleSongDuration, songDuration);
    } else {
        songDuration = Duration::Unknown;
        keys[currentKey++] = MAFW_METADATA_KEY_DURATION;
    }

    if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_IS_SEEKABLE) ))
        onMediaIsSeekable(g_value_get_boolean(v));
    // if the renderer can't determine seekability, there's probably no point in querying the source

    keys[currentKey] = NULL;
    metadataObjectId = objectId;
    metadataSource->setSource(mafwFactory->getTempSource()->getSourceByUUID(objectId.left(objectId.indexOf("::"))));
    metadataSource->getMetadata(objectId.toUtf8(), keys);
    delete[] keys;

    QFont f = ui->titleLabel->font();
    QFontMetrics fm(f);

    ui->titleLabel->setWhatsThis(title);
    if (!this->isActiveWindow())
        this->setWindowTitle(title);
    title = fm.elidedText(title, Qt::ElideRight, 425);
    ui->titleLabel->setText(title);

    ui->artistLabel->setWhatsThis(artist);
    artist = fm.elidedText(artist, Qt::ElideRight, 425);
    ui->artistLabel->setText(artist);

    ui->albumLabel->setWhatsThis(album);
    album = fm.elidedText(album, Qt::ElideRight, 425);
    ui->albumLabel->setText(album);

    ui->trackLengthLabel->setText(mmss_len(songDuration));
    ui->positionSlider->setRange(0, songDuration);

    updateQmlViewMetadata();

    if (!error.isEmpty())
        qDebug() << error;
}

void NowPlayingWindow::onSourceMetadataRequested(QString objectId, GHashTable *metadata, QString error)
{
    if (objectId != metadataObjectId) return;

    if (metadata != NULL) {
        QString albumArt;
        GValue *v;

        QListWidgetItem* item = ui->songList->item(lastPlayingSong->value().toInt());

        QFont f = ui->titleLabel->font();
        QFontMetrics fm(f);

        if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE) )) {
            QString title = QString::fromUtf8(g_value_get_string (v));
            ui->titleLabel->setWhatsThis(title);
            if (!this->isActiveWindow())
                this->setWindowTitle(title);
            title = fm.elidedText(title, Qt::ElideRight, 425);
            ui->titleLabel->setText(title);
            if (item && item->data(UserRoleSongTitle).toString().isEmpty())
                item->setData(UserRoleSongTitle, title);
        }

        if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST) )) {
            QString artist = QString::fromUtf8(g_value_get_string(v));
            ui->artistLabel->setWhatsThis(artist);
            artist = fm.elidedText(artist, Qt::ElideRight, 425);
            ui->artistLabel->setText(artist);
            if (item && item->data(UserRoleSongArtist).toString().isEmpty())
                item->setData(UserRoleSongArtist, artist);
        }

        if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM) )) {
            QString album = QString::fromUtf8(g_value_get_string(v));
            ui->albumLabel->setWhatsThis(album);
            album = fm.elidedText(album, Qt::ElideRight, 425);
            ui->albumLabel->setText(album);
            if (item && item->data(UserRoleSongAlbum).toString().isEmpty())
                item->setData(UserRoleSongAlbum, album);
        }

        if (( v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION) )) {
            songDuration = g_value_get_int (v);
            ui->trackLengthLabel->setText(mmss_len(songDuration));
            ui->positionSlider->setRange(0, songDuration);
            if (item && item->data(UserRoleSongDuration).toInt() == Duration::Blank)
                item->setData(UserRoleSongDuration, songDuration);
        }

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        currentURI = v ? QString::fromUtf8(g_value_get_string (v)) : "";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME);
        currentMIME = v ? QString::fromUtf8(g_value_get_string (v)) : "audio/unknown";

        /*if (item && item->data(UserRoleSongDuration).toInt() == Duration::Blank) {
            if (playlistTime > 0 && item->data(UserRoleSongDuration).toInt() > 0)
                playlistTime -= item->data(UserRoleSongDuration).toInt();
            if (duration > 0)
                playlistTime += duration;
        }*/

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                setAlbumImage(QString::fromUtf8(filename));
        } else {
            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_RENDERER_ART_URI);
            if (v != NULL) {
                const gchar* file_uri = g_value_get_string(v);
                gchar* filename = NULL;
                if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                    setAlbumImage(QString::fromUtf8(filename));
            } else
                setAlbumImage(defaultAlbumImage);
        }

        if (enableLyrics)
            reloadLyrics();

        updateQmlViewMetadata();

        if (!error.isEmpty())
            qDebug() << error;
    }
}

#endif

void NowPlayingWindow::setLyrics(QString lyrics)
{
    ui->lyricsText->setText(lyrics);
}

void NowPlayingWindow::reloadLyrics()
{
    ui->lyricsText->setText(tr("Fetching lyrics..."));
    lyricsManager->fetchLyrics(ui->artistLabel->whatsThis(), ui->titleLabel->whatsThis());
}

void NowPlayingWindow::reloadLyricsOverridingCache()
{
    ui->lyricsText->setText(tr("Fetching lyrics..."));
    lyricsManager->fetchLyrics(ui->artistLabel->whatsThis(), ui->titleLabel->whatsThis(), false);
}

void NowPlayingWindow::editLyrics()
{
    (new EditLyrics(ui->artistLabel->whatsThis(), ui->titleLabel->whatsThis(), this))->show();
}

void NowPlayingWindow::orientationChanged(int w, int h)
{
    if (w > h) { // Landscape mode
        portrait = false;
        ui->orientationLayout->setDirection(QBoxLayout::LeftToRight);
        if (ui->volumeWidget->isHidden())
            ui->volumeWidget->show();

        ui->volumeWidget->setContentsMargins(0,0,9,9);
        ui->buttonsLayout->setSpacing(60);

        ui->titleLabel->setAlignment(Qt::AlignLeft);
        ui->artistLabel->setAlignment(Qt::AlignLeft);
        ui->albumLabel->setAlignment(Qt::AlignLeft);
        ui->lyricsText->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

        ui->view_small->hide();
        ui->view_large->show();

    } else { // Portrait mode
        portrait = true;
        ui->orientationLayout->setDirection(QBoxLayout::TopToBottom);
        if (!ui->volumeButton->isHidden())
            ui->volumeWidget->hide();

        ui->volumeWidget->setContentsMargins(18,0,9,9);
        ui->buttonsLayout->setSpacing(27);

        ui->titleLabel->setAlignment(Qt::AlignHCenter);
        ui->artistLabel->setAlignment(Qt::AlignHCenter);
        ui->albumLabel->setAlignment(Qt::AlignHCenter);
        ui->lyricsText->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        if (ui->infoWidget->isHidden()) {
            ui->view_large->hide();
            ui->view_small->show();
        }
    }
}

void NowPlayingWindow::toggleList()
{
    if (ui->songList->isHidden() && ui->lyricsArea->isHidden()) {
        // Playlist view
        if (portrait) {
            ui->view_large->hide();
            ui->view_small->show();
        }
        ui->infoWidget->hide();
        ui->lyricsArea->hide();
        ui->songList->show();
        ui->songList->setFocus();
#ifdef MAFW
        positionTimer->stop();
#endif
    } else if (enableLyrics && ui->lyricsArea->isHidden() && ui->infoWidget->isHidden()) {
        // Lyrics view
        ui->infoWidget->hide();
        ui->songList->hide();
        ui->lyricsArea->show();
        ui->lyricsArea->setFocus();
        if (portrait) {
            ui->view_large->hide();
            ui->view_small->show();
        }
#ifdef MAFW
        positionTimer->stop();
#endif
    } else {
        // Song info view
        ui->lyricsArea->hide();
        ui->songList->hide();
        ui->infoWidget->show();
        if (portrait) {
            ui->view_small->hide();
            ui->view_large->show();
        }
#ifdef MAFW
        if (!positionTimer->isActive() && this->mafwState == Playing) {
            positionTimer->start();
            mafwrenderer->getPosition();
        }
#endif
    }
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

#ifdef MAFW
    playlist->setShuffled(checked);
#endif
}

void NowPlayingWindow::onRepeatButtonToggled(bool checked)
{
    if (checked) {
        ui->repeatButton->setIcon(QIcon(repeatButtonPressedIcon));
    } else {
        ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    }

#ifdef MAFW
    playlist->setRepeat(checked);
#endif
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
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

void NowPlayingWindow::onVolumeSliderReleased()
{
    volumeTimer->start();
#ifdef MAFW
    mafwrenderer->setVolume(ui->volumeSlider->value());
#endif
}

void NowPlayingWindow::onPositionSliderPressed()
{
    onPositionSliderMoved(ui->positionSlider->value());
}

void NowPlayingWindow::onPositionSliderReleased()
{
#ifdef MAFW
    mafwrenderer->setPosition(SeekAbsolute, ui->positionSlider->value());
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? ui->positionSlider->value()-songDuration :
                                                             ui->positionSlider->value()));
#endif
}

void NowPlayingWindow::onPositionSliderMoved(int position)
{
    ui->currentPositionLabel->setText(mmss_pos(reverseTime ? position-songDuration : position));
#ifdef MAFW
    if (!lazySliders)
        mafwrenderer->setPosition(SeekAbsolute, position);
#endif
}

#ifdef MAFW
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
    if (!this->playlistRequested) {
        this->updatePlaylist();
        this->updatePlaylistState();
        this->playlistRequested = true;
    }

    lastPlayingSong->set((int)index); // sometimes the value is wrong, noticable mainly while using the playlist editor
    setSongNumber(index+1, playlist->getSize());
    selectItemByRow(index);
    onStateChanged(state);
}

void NowPlayingWindow::setPosition(int newPosition)
{
    mafwrenderer->setPosition(SeekAbsolute, newPosition);
    mafwrenderer->getPosition();
}

void NowPlayingWindow::showEvent(QShowEvent *)
{
    mafwrenderer->getStatus();
    this->updatePlaylistState();
    this->setWindowTitle(defaultWindowTitle); // avoid showing a different title for a split second
    if (positionTimer->isActive())
        ui->positionSlider->setEnabled(true);
}

void NowPlayingWindow::onGconfValueChanged()
{
    this->setSongNumber(lastPlayingSong->value().toInt()+1, ui->songList->count());
    this->selectItemByRow(lastPlayingSong->value().toInt());
}

void NowPlayingWindow::onMediaChanged(int index, char*)
{
    lastPlayingSong->set(index);
    focusItemByRow(index);
    mafwrenderer->getCurrentMetadata();
}

void NowPlayingWindow::onMediaIsSeekable(bool seekable)
{
    ui->positionSlider->setEnabled(seekable);
    this->isMediaSeekable = seekable;
}
#endif

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

void NowPlayingWindow::togglePlayback()
{
#ifdef MAFW
    if (mafwState == Playing)
        mafwrenderer->pause();
    else if (mafwState == Paused)
        mafwrenderer->resume();
    else if (mafwState == Stopped)
        mafwrenderer->play();
#endif
}

#ifdef MAFW
void NowPlayingWindow::onGetPlaylistItems(QString objectId, GHashTable *metadata, guint index)
{
    QListWidgetItem *item =  ui->songList->item(index);

    if (!item) return; // in case of query manager's outdated request

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

        if (playlistTime > 0 && item->data(UserRoleSongDuration).toInt() > 0)
            playlistTime -= item->data(UserRoleSongDuration).toInt();
        if (duration > 0)
            playlistTime += duration;
        this->updatePlaylistTimeLabel();

        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongIndex, index);
    } else {
        item->setData(UserRoleSongTitle, tr("Information not available"));
        item->setData(UserRoleSongDuration, Duration::Blank);
    }

    if (qmlView) qmlView->setPlaylistItem(index, item);
}

void NowPlayingWindow::onPlaylistItemActivated(QListWidgetItem *item)
{
#ifdef DEBUG
    qDebug() << "Selected item number: " << ui->songList->currentRow();
#endif
    this->setSongNumber(ui->songList->currentRow()+1, ui->songList->count());
    lastPlayingSong->set(ui->songList->currentRow());

    QFont f = ui->titleLabel->font();
    QFontMetrics fm(f);

    ui->titleLabel->setWhatsThis(item->data(UserRoleSongTitle).toString());
    if (!this->isActiveWindow())
        this->setWindowTitle(item->data(UserRoleSongTitle).toString());
    QString elided = fm.elidedText(item->data(UserRoleSongTitle).toString(), Qt::ElideRight, 425);
    ui->titleLabel->setText(elided);

    ui->artistLabel->setWhatsThis(item->data(UserRoleSongArtist).toString());
    elided = fm.elidedText(item->data(UserRoleSongArtist).toString(), Qt::ElideRight, 425);
    ui->artistLabel->setText(elided);

    ui->albumLabel->setWhatsThis(item->data(UserRoleSongAlbum).toString());
    elided = fm.elidedText(item->data(UserRoleSongAlbum).toString(), Qt::ElideRight, 425);
    ui->albumLabel->setText(elided);

    ui->currentPositionLabel->setText(mmss_pos(0));
    this->songDuration = item->data(UserRoleSongDuration).toInt();
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
        playlistTime = 0;
        playlist->clear();
        lastPlayingSong->set(1);
        this->close();
    }
}
#endif

void NowPlayingWindow::onContextMenuRequested(const QPoint &pos)
{
    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Delete from now playing"), this, SLOT(onDeleteFromNowPlaying()));
    if (!ui->songList->currentItem()->data(UserRoleObjectID).toString().startsWith("_uuid_")) {
        if (permanentDelete) contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
        contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
        contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
        contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    }
    contextMenu->exec(this->mapToGlobal(pos));
}

void NowPlayingWindow::onAddToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
#ifdef MAFW
        playlist->appendItem(picker.playlist, ui->songList->currentItem()->data(UserRoleObjectID).toString());
#endif
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
#endif
    }
}

void NowPlayingWindow::setRingingTone()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Ringtone, this,
                      ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                      ui->songList->currentItem()->data(UserRoleSongTitle).toString())
        .exec() == QMessageBox::Yes)
    {
        mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
    }
#endif
}

#ifdef MAFW
void NowPlayingWindow::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

#ifdef Q_WS_MAEMO_5
    QDBusInterface setRingtone("com.nokia.profiled",
                               "/com/nokia/profiled",
                               "com.nokia.profiled",
                               QDBusConnection::sessionBus(), this);
    setRingtone.call("set_value", "general", "ringing.alert.tone", uri);
    QMaemo5InformationBox::information(this, tr("Selected song set as ringing tone"));
#endif
}
#endif

void NowPlayingWindow::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::DeleteSong, this,
                      ui->songList->currentItem()->data(UserRoleSongTitle).toString(),
                      ui->songList->currentItem()->data(UserRoleSongArtist).toString())
        .exec() == QMessageBox::Yes)
    {
        playlist->removeItem(ui->songList->currentRow());
        mafwTrackerSource->destroyObject(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    }
#endif
}

void NowPlayingWindow::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void NowPlayingWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QStringList files;
#ifdef DEBUG
    qDebug() << "Sending file:" << uri;
#endif
    files.append(uri);
#ifdef Q_WS_MAEMO_5
    ShareDialog(files, this).exec();
#endif
}
#endif

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
        qmlView = new QmlView(source, this, mafwFactory);
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
#ifdef MAFW
    mafwrenderer->getPosition();
#endif
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

#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", songCount));
#endif
    }
}

void NowPlayingWindow::onDeleteFromNowPlaying()
{
#ifdef MAFW
    playlist->removeItem(ui->songList->currentRow());
#endif
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

    bool synthetic = from == -1;

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
            if (playlistTime > 0 && item->data(UserRoleSongDuration).toInt() > 0)
                playlistTime -= item->data(UserRoleSongDuration).toInt();
            delete item;

            if (qmlView) qmlView->removePlaylistItem(from);
        }
        this->updatePlaylistTimeLabel();
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

    this->setSongNumber(lastPlayingSong->value().toInt()+1, ui->songList->count());

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
        setAlbumImage(MediaArt::setAlbumImage(ui->albumLabel->whatsThis(), picker.cover));
}

void NowPlayingWindow::resetAlbumArt()
{
    if (ConfirmDialog(ConfirmDialog::ResetArt, this).exec() == QMessageBox::Yes) {
        // Remove old art using MediaArt::setAlbumImage() and let
        // NowPlayingWindow::setAlbumImage() search for covers.
        setAlbumImage(MediaArt::setAlbumImage(ui->albumLabel->text(), ""));

#ifdef Q_WS_MAEMO_5
        // Even if NowPlayingWindow::setAlbumImage() falls back to the default
        // art, there still can be the embedded image, so poke Tracker to
        // recheck the song file.
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
            QTimer::singleShot(3000, this, SLOT(refreshAlbumArt()));
        }
#endif
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
    contextMenu->addAction(tr("Reload lyrics"), this, SLOT(reloadLyricsOverridingCache()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void NowPlayingWindow::closeEvent(QCloseEvent *e)
{
    this->hide();
    this->setParent(0);
    emit hidden();
    e->ignore();
}
