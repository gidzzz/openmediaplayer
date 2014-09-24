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

#include "mainwindow.h"

QString defaultAlbumImage;
QString defaultRadioImage;
QString volumeButtonIcon;

MainWindow::MainWindow(QWidget *parent) :
    BaseWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPalette palette;
#ifdef Q_WS_MAEMO_5
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif
    palette.setColor(QPalette::WindowText, secondaryColor);

    ui->songCountLabel->setPalette(palette);
    ui->videoCountLabel->setPalette(palette);
    ui->stationCountLabel->setPalette(palette);

    loadThemeIcons();
    setButtonIcons();

    ui->menuList->setItemDelegate(new MainDelegate(ui->menuList));

#ifdef MAFW
    mafwRegistry = MafwRegistryAdapter::get();
    mafwrenderer = mafwRegistry->renderer();
    mafwTrackerSource = mafwRegistry->source(MafwRegistryAdapter::Tracker);
    mafwRadioSource = mafwRegistry->source(MafwRegistryAdapter::Radio);
    playlist = mafwRegistry->playlist();

    MissionControl::acquire()->setRegistry(mafwRegistry);
#endif

#ifdef Q_WS_MAEMO_5
#else
    // Menu bar breaks layouts on desktop, hide it.
    ui->menuBar->hide();
#endif

#ifdef MAFW
    musicWindow = new MusicWindow(this, mafwRegistry);
#else
    musicWindow = new MusicWindow(this);
#endif

    connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(onOrientationChanged(int,int)));
    rotator->setSlave(this);
    reloadSettings();

#ifdef MAFW
    ui->upnpControl->setRegistry(mafwRegistry);

    ui->indicator->setRegistry(mafwRegistry);
#endif
    ui->indicator->setFixedWidth(112);
    ui->indicator->setFixedHeight(70);

    connect(mafwRadioSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    if (mafwRadioSource->isReady())
        onContainerChanged(mafwRadioSource->uuid() + "::");

    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    if (mafwTrackerSource->isReady()) {
        onContainerChanged(mafwTrackerSource->uuid() + "::");
        registerDbusService();
    } else {
        connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(registerDbusService()));
    }

#ifdef Q_WS_MAEMO_5
    updatingShow = true;

    updatingProgressBar = new QProgressBar;
    updatingProgressBar->setTextVisible(false);

    updatingLabel = new QLabel;
    updatingLabel->setAlignment(Qt::AlignCenter);
    updatingLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 15, 0, 15);
    layout->addWidget(updatingLabel);
    layout->addWidget(updatingProgressBar);

    QWidget *updatingWidget = new QWidget;
    updatingWidget->setLayout(layout);

    updatingInfoBox = new QMaemo5InformationBox(this);
    updatingInfoBox->setTimeout(0);
    updatingInfoBox->setWidget(updatingWidget);

    QTimer::singleShot(1000, this, SLOT(takeScreenshot()));
#endif
}

MainWindow::~MainWindow()
{
    mafwrenderer->enablePlayback(false);

    QString action = QSettings().value("main/onApplicationExit", "stop-playback").toString();

    if (action == "stop-playback")
        mafwrenderer->stop();
    else if (action == "pause-playback")
        mafwrenderer->pause();

    delete ui;
}

#ifdef Q_WS_MAEMO_5
void MainWindow::registerDbusService()
{
    disconnect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(registerDbusService()));

    if (!QDBusConnection::sessionBus().registerService(DBUS_SERVICE))
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));

    if (!QDBusConnection::sessionBus().registerObject(DBUS_PATH, this, QDBusConnection::ExportScriptableSlots))
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));

    if (!QDBusConnection::sessionBus().registerObject("/com/nokia/mediaplayer", this, QDBusConnection::ExportScriptableSlots))
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
}
#endif

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage(backgroundImage));
}

void MainWindow::loadThemeIcons()
{
    QString theme = QSettings("/etc/hildon/theme/index.theme", QSettings::IniFormat)
                    .value("X-Hildon-Metatheme/IconTheme","hicolor")
                    .toString();

    defaultAlbumImage = QFileInfo("/usr/share/icons/"+theme+"/295x295/hildon/mediaplayer_default_album.png").exists()
                      ? "/usr/share/icons/"+theme+"/295x295/hildon/mediaplayer_default_album.png"
                      : "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png";

    defaultRadioImage = QFileInfo("/usr/share/icons/"+theme+"/295x295/hildon/mediaplayer_default_stream.png").exists()
                      ? "/usr/share/icons/"+theme+"/295x295/hildon/mediaplayer_default_stream.png"
                      : "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_stream.png";

    volumeButtonIcon = QFileInfo("/usr/share/icons/"+theme+"/64x64/hildon/mediaplayer_volume.png").exists()
                     ? "/usr/share/icons/"+theme+"/64x64/hildon/mediaplayer_volume.png"
                     : "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png";
}

void MainWindow::setButtonIcons()
{
    ui->musicButton->setIcon(QIcon::fromTheme(musicIcon));
    ui->videosButton->setIcon(QIcon::fromTheme(videosIcon));
    ui->radioButton->setIcon(QIcon::fromTheme(radioIcon));
    ui->shuffleAllButton->setIcon(QIcon::fromTheme(shuffleIcon));

    ui->menuList->item(0)->setIcon(QIcon::fromTheme(musicIcon));
    ui->menuList->item(1)->setIcon(QIcon::fromTheme(videosIcon));
    ui->menuList->item(2)->setIcon(QIcon::fromTheme(radioIcon));
    ui->menuList->item(3)->setIcon(QIcon::fromTheme(shuffleIcon));
}

void MainWindow::connectSignals()
{
    connect(new QShortcut(QKeySequence(Qt::Key_Q), this), SIGNAL(activated()), this, SLOT(showMusicWindow()));
    connect(new QShortcut(QKeySequence(Qt::Key_W), this), SIGNAL(activated()), this, SLOT(showVideosWindow()));
    connect(new QShortcut(QKeySequence(Qt::Key_E), this), SIGNAL(activated()), this, SLOT(showInternetRadioWindow()));
    connect(new QShortcut(QKeySequence(Qt::Key_R), this), SIGNAL(activated()), this, SLOT(onShuffleAllClicked()));

    connect(ui->musicButton, SIGNAL(clicked()), this, SLOT(showMusicWindow()));
    connect(ui->videosButton, SIGNAL(clicked()), this, SLOT(showVideosWindow()));
    connect(ui->radioButton, SIGNAL(clicked()), this, SLOT(showInternetRadioWindow()));
    connect(ui->shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleAllClicked()));

    connect(ui->menuList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));

    connect(ui->upnpControl, SIGNAL(childOpened()), this, SLOT(onChildOpened()));
    connect(ui->upnpControl, SIGNAL(childClosed()), this, SLOT(onChildClosed()));

    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->actionSleeper, SIGNAL(triggered()), this, SLOT(openSleeperDialog()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(musicWindow, SIGNAL(hidden()), this, SLOT(onChildClosed()));

#ifdef MAFW
    connect(mafwRadioSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countRadioResult(QString, GHashTable*, QString)));

    connect(mafwTrackerSource, SIGNAL(updating(int,int,int,int)), this, SLOT(onSourceUpdating(int,int,int,int)));
    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countAudioVideoResult(QString, GHashTable*, QString)));
#endif
}

void MainWindow::open_mp_main_view()
{
    closeChildren();
}

void MainWindow::open_mp_now_playing()
{
    // maybe this check could be moved to NowPlayingWindow?
    if (mafwrenderer->isRendererReady() && mafwTrackerSource->isReady() && !playlist->isPlaylistNull()) {
#ifdef MAFW
        if (playlist->playlistName() != "FmpAudioPlaylist")
            playlist->assignAudioPlaylist();
#endif
        createNowPlayingWindow();
    } else {
        QTimer::singleShot(1000, this, SLOT(open_mp_now_playing()));
    }
}

void MainWindow::open_mp_radio_playing()
{
    closeChildren();

    if (playlist->playlistName() != "FmpRadioPlaylist")
        playlist->assignRadioPlaylist();

    RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this, mafwRegistry);
    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    window->show();
    ui->indicator->inhibit();
}

void MainWindow::open_mp_now_playing_playback_on()
{
    open_mp_now_playing();
}

void MainWindow::open_mp_radio_playing_playback_on()
{
    open_mp_radio_playing();
}

void MainWindow::open_mp_car_view()
{
    createNowPlayingWindow()->showCarView();
}

#ifdef MAFW
void MainWindow::openDirectory(QString uri, Media::Type type)
{
    QString path = QString::fromUtf8(g_filename_from_uri(uri.left(uri.lastIndexOf('/')+1).toUtf8(), NULL, NULL));

    QString root;
    QString filter;
    if (type == Media::Audio) {
        root = "localtagfs::music/songs/";
        filter = "audio";
    } else { // type == Media::Video
        root = "localtagfs::videos/";
        filter = "video";
    }

    QDir dir(path);
    dir.setFilter(QDir::Files);
    QStringList entries = dir.entryList();

    int songAddBufferSize = entries.size();
    gchar** songAddBuffer = new gchar*[songAddBufferSize+1];
    int songAddBufferPos = 0;

    for (int i = 0; i < songAddBufferSize; i++) {
        QString file = entries.at(i);
        QString mime = gnome_vfs_get_mime_type_for_name(file.toUtf8());

        // maybe there's a better way to ignore playlists
        if (mime.startsWith(filter) && !mime.endsWith("mpegurl"))
            songAddBuffer[songAddBufferPos++] = qstrdup(file.prepend(path)
                                                            .replace("/", "%2F")
                                                            .prepend(root)
                                                            .toUtf8());
    }

    songAddBuffer[songAddBufferPos] = NULL;

    if (type == Media::Audio) {
        if (playlist->playlistName() != "FmpAudioPlaylist")
            playlist->assignAudioPlaylist();
        if (!QSettings().value("main/appendSongs").toBool())
            playlist->clear();
    } else { // type == Media::Video
        playlist->assignVideoPlaylist();
        playlist->clear();
    }

    playlist->appendItems((const gchar**)songAddBuffer);

    if (type == Media::Audio) {
        if (!QSettings().value("main/appendSongs").toBool()) {
            for (int i = 0; i < songAddBufferPos; i++) {
                if (QString::fromUtf8(songAddBuffer[i]) == objectIdToPlay) {
                    mafwrenderer->gotoIndex(i);
                    mafwrenderer->play();
                    break;
                }
            }
        }
        createNowPlayingWindow();
    } else { // type == Media::Video
        for (int i = 0; i < songAddBufferPos; i++) {
            if (QString::fromUtf8(songAddBuffer[i]) == objectIdToPlay) {
                mafwrenderer->gotoIndex(i);
                break;
            }
        }
        createVideoNowPlayingWindow();
    }

    for (int i = 0; i < songAddBufferPos; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}
#endif

void MainWindow::mime_open(const QString &uriString)
{
#ifdef MAFW
    QString uriToPlay = uriString.startsWith("/") ? "file://" + uriString : uriString;
    QString objectId = mafwTrackerSource->createObjectId(uriToPlay);
    qDebug() << "ID to open:" << objectId;

    // Local resource
    if (uriToPlay.startsWith("file://")) {
        QString mime(gnome_vfs_get_mime_type_for_name(uriToPlay.toUtf8()));
        qDebug() << "MIME:" << mime;

        // Audio or a playlist
        if (mime.startsWith("audio")) {

            // M3U playlist, browse contents
            if (mime.endsWith("mpegurl")) {
#ifdef Q_WS_MAEMO_5
                setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
                objectId.remove(0, 18) // "urisource::file://"
                        .replace("/", "%2F")
                        .prepend(TAGSOURCE_PLAYLISTS_PATH + QString("/"));
                qDebug() << "Converted ID:" << objectId;

                CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwRegistry);
                connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAddFinished(uint)), Qt::UniqueConnection);
                playlistToken = cpm->appendBrowsed(objectId, QString(), QString(), MAFW_SOURCE_BROWSE_ALL, true);
            }

            else {
                // Allow the user to select the opening mode
                if (QSettings().value("main/showOpenDialog", true).toBool()) {
                    closeChildren();
                    if (OpenDialog(this).exec() == QDialog::Rejected)
                        return;
                }

                // Audio, a whole directory has to be added
                if (QSettings().value("main/openFolders").toBool()) {
#ifdef Q_WS_MAEMO_5
                    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
                    objectIdToPlay = objectId.remove(0, 18) // "urisource::file://"
                                            .replace("/", "%2F")
                                            .prepend(TAGSOURCE_AUDIO_PATH + QString("/"));
                    qDebug() << "Converted ID:" << objectIdToPlay;

                    openDirectory(uriToPlay, Media::Audio);
                }

                // Audio, just one file
                else {
                    objectId.remove(0, 18) // "urisource::file://"
                            .replace("/", "%2F")
                            .prepend(TAGSOURCE_AUDIO_PATH + QString("/"));
                    qDebug() << "Converted ID:" << objectId;

                    if (playlist->playlistName() != "FmpAudioPlaylist")
                        playlist->assignAudioPlaylist();
                    if (!QSettings().value("main/appendSongs").toBool())
                        playlist->clear();
                    playlist->appendItem(objectId);

                    if (!QSettings().value("main/appendSongs").toBool()) {
                        if (mafwrenderer->isRendererReady())
                            mafwrenderer->play();
                        else
                            connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(play()));
                    }

                    createNowPlayingWindow();
                }
            }
        }

        // Video only
        else if (mime.startsWith("video")) {
            // Allow the user to select the opening mode
            if (QSettings().value("main/showOpenDialog", true).toBool()) {
                closeChildren();
                if (OpenDialog(this, true).exec() == QDialog::Rejected)
                    return;
            }

            // A whole directory has to be added
            if (QSettings().value("main/openFolders").toBool()) {
#ifdef Q_WS_MAEMO_5
                setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
                objectIdToPlay = objectId.remove(0, 18) // "urisource::file://"
                                         .replace("/", "%2F")
                                         .prepend(TAGSOURCE_VIDEO_PATH + QString("/"));
                qDebug() << "Converted ID:" << objectIdToPlay;

                openDirectory(uriToPlay, Media::Video);
            }

            // Just one file
            else {
                objectId = objectId.remove(0, 18) // "urisource::file://"
                                   .replace("/", "%2F")
                                   .prepend(TAGSOURCE_VIDEO_PATH + QString("/"));
                qDebug() << "Converted ID:" << objectId;

                playlist->assignVideoPlaylist();
                playlist->clear();
                playlist->appendItem(objectId);
                createVideoNowPlayingWindow();
            }
        }
    }

    // Remote resource, use the video window for media type detection
    else {
        playlist->assignVideoPlaylist();
        playlist->clear();
        playlist->appendItem(objectId);
        createVideoNowPlayingWindow();
    }
#endif
}

void MainWindow::play_automatic_playlist(const QString &playlistName, bool shuffle)
{
#ifdef MAFW
    QString filter;
    QString sorting;
    int limit = QSettings().value("music/playlistSize", 30).toInt();

    if (playlistName == "recently-added") {
        filter = ""; sorting = "-added";
    } else if (playlistName == "recently-played") {
        filter = "(play-count>0)"; sorting = "-last-played";
    } else if (playlistName == "most-played") {
        filter = "(play-count>0)"; sorting = "-play-count,+title";
    } else if (playlistName == "never-played") {
        filter = "(play-count=)"; sorting = ""; limit = MAFW_SOURCE_BROWSE_ALL;
    } else {
        return;
    }

    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);

    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(shuffle);

    CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwRegistry);
    connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAddFinished(uint)), Qt::UniqueConnection);
    playlistToken = cpm->appendBrowsed("localtagfs::music/songs", filter, sorting, limit);
#endif
}

void MainWindow::play_saved_playlist(const QString &playlistName, bool shuffle)
{
#ifdef MAFW
    MafwPlaylistManagerAdapter *mafwPlaylistManager = MafwPlaylistManagerAdapter::get();

    GArray* playlists = mafwPlaylistManager->listPlaylists();

    if (playlists->len != 0) {
        for (uint i = 0; i < playlists->len; i++) {
            if (playlistName == QString::fromUtf8(g_array_index(playlists, MafwPlaylistManagerItem, i).name)) {
                setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

                if (playlist->playlistName() != "FmpAudioPlaylist")
                    playlist->assignAudioPlaylist();
                playlist->clear();
                playlist->setShuffled(shuffle);

                MafwPlaylist *mafwplaylist = MAFW_PLAYLIST(mafwPlaylistManager->createPlaylist(playlistName));
                gchar** items = mafw_playlist_get_items(mafwplaylist, 0, playlist->getSizeOf(mafwplaylist)-1, NULL);
                playlist->appendItems((const gchar**)items);
                g_strfreev(items);

                mafwrenderer->play();
                createNowPlayingWindow();
                setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
            }
        }
    }

    mafwPlaylistManager->freeListOfPlaylists(playlists);
#endif
}

void MainWindow::top_application()
{
    raise();
    activateWindow();
}

NowPlayingWindow* MainWindow::createNowPlayingWindow()
{
    closeChildren();

#ifdef MAFW
    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwRegistry);
#else
    NowPlayingWindow *window = NowPlayingWindow::acquire(this);
#endif
    window->show();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();

    return window;
}

void MainWindow::createVideoNowPlayingWindow()
{
    closeChildren();

#ifdef MAFW
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwRegistry);
#else
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
    window->showFullScreen();

    connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(restore()));
    ui->indicator->inhibit();

#ifdef MAFW
    window->play();
#endif
}

void MainWindow::onOrientationChanged(int w, int h)
{
    if (w > h) { // Landscape
        ui->menuList->hide();
        ui->menuWidget->show();
    } else { // Portrait
        ui->menuWidget->hide();
        ui->menuList->show();
    }

    ui->bottomWidget->setGeometry(0, h-56-70, w-8, 70);

    ui->bottomWidget->raise();
}

void MainWindow::showAbout()
{
    AboutWindow *about = new AboutWindow(this);
    about->show();
}

void MainWindow::processListClicks(QListWidgetItem *item)
{
    switch (ui->menuList->row(item)) {
        case 0: showMusicWindow(); break;
        case 1: showVideosWindow(); break;
        case 2: showInternetRadioWindow(); break;
        case 3: onShuffleAllClicked(); break;
    }
}

void MainWindow::openSettings()
{
    SettingsDialog *settings = new SettingsDialog(this);
    connect(settings, SIGNAL(accepted()), this, SLOT(reloadSettings()));
    settings->show();
}

void MainWindow::reloadSettings()
{
    NowPlayingWindow::destroy();

    MissionControl::acquire()->reloadSettings();

    musicWindow->refreshPlaylistView();

    QString orientation = QSettings().value("main/orientation").toString();
    Rotator::acquire()->setPolicy(orientation == "landscape" ? Rotator::Landscape :
                                  orientation == "portrait"  ? Rotator::Portrait  :
                                                               Rotator::Automatic);

#ifdef MAFW
    if (mafwrenderer->isRendererReady())
        setupPlayback();
    else
        connect(mafwrenderer, SIGNAL(rendererReady()), this, SLOT(setupPlayback()));
#endif
}

void MainWindow::openSleeperDialog()
{
    (new SleeperDialog(this))->show();
}

void MainWindow::showMusicWindow()
{
    this->setEnabled(false);
    musicWindow->setEnabled(true);

    musicWindow->show();

    ui->indicator->inhibit();
}

void MainWindow::showVideosWindow()
{
    this->setEnabled(false);

#ifdef MAFW
    VideosWindow *window = new VideosWindow(this, mafwRegistry);
#else
    VideosWindow *window = new VideosWindow(this);
#endif

    window->show();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}

void MainWindow::showInternetRadioWindow()
{
    this->setEnabled(false);

#ifdef MAFW
    InternetRadioWindow *window = new InternetRadioWindow(this, mafwRegistry);
#else
    InternetRadioWindow *window = new InternetRadioWindow(this);
#endif

    window->show();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}

#ifdef MAFW
void MainWindow::countSongs()
{
    mafwTrackerSource->getMetadata(TAGSOURCE_AUDIO_PATH,
                                   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1));
}

void MainWindow::countVideos()
{
    mafwTrackerSource->getMetadata(TAGSOURCE_VIDEO_PATH,
                                   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1));
}

void MainWindow::countRadioStations()
{
    mafwRadioSource->getMetadata(RADIOSOURCE_PATH,
                                 MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1));
}


void MainWindow::countAudioVideoResult(QString objectId, GHashTable* metadata, QString error)
{
    GValue *v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
    int count = v ? g_value_get_int(v) : -1;

    if (objectId == TAGSOURCE_AUDIO_PATH) {
        QString countStr = (count == -1) ? tr("(no songs)") :
                                           tr("%n song(s)", "", count);

        ui->songCountLabel->setText(countStr);
        ui->menuList->item(0)->setData(UserRoleValueText, countStr);

    } else if (objectId == TAGSOURCE_VIDEO_PATH) {
        QString countStr = (count == -1) ? tr("(no videos)") :
                                           tr("%n clip(s)", "", count);

        ui->videoCountLabel->setText(countStr);
        ui->menuList->item(1)->setData(UserRoleValueText, countStr);
    }

    if (!error.isEmpty())
        qDebug() << error;
}

void MainWindow::countRadioResult(QString, GHashTable *metadata, QString error)
{
    GValue *v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
    int count = v ? g_value_get_int(v) : -1;

    QString countStr = (count == -1) ? tr("(no stations)") :
                                       tr("%n station(s)", "", count);

    ui->stationCountLabel->setText(countStr);
    ui->menuList->item(2)->setData(UserRoleValueText, countStr);

    if (!error.isEmpty())
        qDebug() << error;
}

void MainWindow::onAddFinished(uint token)
{
    if (token != playlistToken) return;

    if (playlist->playlistName() == "FmpAudioPlaylist") {
        mafwrenderer->play();
        createNowPlayingWindow();
    } else {
        createVideoNowPlayingWindow();
    }

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}
#endif

void MainWindow::onShuffleAllClicked()
{
#ifdef MAFW
    this->setEnabled(false);

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(true);

    CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwRegistry);
    connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAddFinished(uint)), Qt::UniqueConnection);
    playlistToken = cpm->appendBrowsed("localtagfs::music/songs");
#endif
}

#ifdef MAFW
void MainWindow::onSourceUpdating(int progress, int processed_items, int remaining_items, int remaining_time)
{
    qDebug() << "MainWindow::onSourceUpdating("
             << "progress =" << progress
             << ", processed_items =" << processed_items
             << ", remaining_items =" << remaining_items
             << ", remaining_time =" << remaining_time << ")";

    QString text;
    text.append(tr("Retrieving information on the new media files"));
    text.append("\n");
    text.append(tr("Estimated time remaining:") + " " + (remaining_time < 0 ? "?" : mmss_len(remaining_time)));
    text.append("\n");
    text.append(tr("Remaining items:") + " " + (remaining_items < 0 ? "?" : QString::number(remaining_items)));

#ifdef Q_WS_MAEMO_5
    updatingLabel->setText(text);
    updatingProgressBar->setValue(progress);

    // Update signal is not only emitted during a general Tracker update, but also
    // in other cases (after deleting and often after selecting a song to play,
    // for example). Update notifications showing up for no apparent reason are
    // rather annoying, so it's better to ignore signals leading to such situation.
    // What they have in common seems to be remaining_items=0.
    if (remaining_items > 0 && updatingShow) {
        updatingInfoBox->show();
        updatingShow = false;
    } else if (progress == 100) {
        updatingInfoBox->hide();
        updatingShow = true;
    }
#endif
}

#ifdef Q_WS_MAEMO_5
void MainWindow::onScreenLocked(bool locked)
{
#ifdef MAFW
    if (locked)
        mafwrenderer->enablePlayback(QSettings().value("main/managedPlayback") == "locked",
                                     QSettings().value("main/compatiblePlayback", true).toBool());
    else
        mafwrenderer->enablePlayback(QSettings().value("main/managedPlayback") == "unlocked",
                                     QSettings().value("main/compatiblePlayback", true).toBool());
#endif
}
#endif

#ifdef MAFW
void MainWindow::setupPlayback()
{
    disconnect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));

    QString playback = QSettings().value("main/managedPlayback", "always").toString();
    if (playback == "always")
        mafwrenderer->enablePlayback(true, QSettings().value("main/compatiblePlayback", true).toBool());
    else if (playback == "never")
        mafwrenderer->enablePlayback(false);
    else if (playback == "locked" || playback == "unlocked") {
        connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));
        onScreenLocked(Maemo5DeviceEvents::acquire()->isScreenLocked());
    }
}
#endif

void MainWindow::onContainerChanged(QString objectId)
{
    if (objectId.startsWith("localtagfs::")) {
        const QString category = objectId.mid(12);
        if (category.isEmpty() || category.startsWith("music"))
            countSongs();
        if (category.isEmpty() || category.startsWith("videos"))
            countVideos();
    } else if (objectId.startsWith("iradiosource::")) {
        countRadioStations();
    }
}
#endif

#ifdef Q_WS_MAEMO_5
void MainWindow::takeScreenshot()
{
    // True takes a screenshot, false destroys it
    // See http://maemo.org/api_refs/5.0/5.0-final/hildon/hildon-Additions-to-GTK+.html#hildon-gtk-window-take-screenshot
    bool take = true;
    XEvent xev = { 0 };

    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = QX11Info::display();
    xev.xclient.window = XDefaultRootWindow (xev.xclient.display);
    xev.xclient.message_type = XInternAtom (xev.xclient.display, "_HILDON_LOADING_SCREENSHOT", False);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = take ? 0 : 1;
    xev.xclient.data.l[1] = this->winId();

    XSendEvent (xev.xclient.display,
                xev.xclient.window,
                False,
                SubstructureRedirectMask | SubstructureNotifyMask,
                &xev);

    XFlush (xev.xclient.display);
    XSync (xev.xclient.display, False);
}
#endif

void MainWindow::closeChildren()
{
    QList<QDialog*> dialogs = findChildren<QDialog*>();
    for (int i = 0; i < dialogs.size(); i++)
        dialogs.at(i)->close();

    QList<QMainWindow*> windows = findChildren<QMainWindow*>();
    for (int i = 0; i < windows.size(); i++)
        windows.at(i)->close();
}

void MainWindow::onChildOpened()
{
    this->setEnabled(false);
    ui->indicator->inhibit();
}

void MainWindow::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    this->onChildClosed();
}

void MainWindow::onChildClosed()
{
    ui->indicator->restore();
    ui->menuList->clearSelection();
    this->setEnabled(true);
}
