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
#include "delegates/maintdelegate.h"
#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#define HAL_PATH_RX51_JACK "/org/freedesktop/Hal/devices/platform_soc_audio_logicaldev_input"
#endif

QSettings settings( "/etc/hildon/theme/index.theme", QSettings::IniFormat );
QString currtheme = settings.value("X-Hildon-Metatheme/IconTheme","hicolor").toString();
QString albumImage, radioImage;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->songCountL->setText(tr("%n song(s)", "", 0));
    ui->videoCountL->setText(tr("%n clip(s)", "", 0));
    ui->stationCountL->setText(tr("%n station(s)", "", 0));
#ifdef Q_WS_MAEMO_5
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
#else
    QColor secondaryColor(156, 154, 156);
#endif
    ui->songCountL->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                              .arg(secondaryColor.red())
                              .arg(secondaryColor.green())
                              .arg(secondaryColor.blue()));
    ui->videoCountL->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                              .arg(secondaryColor.red())
                              .arg(secondaryColor.green())
                              .arg(secondaryColor.blue()));
    ui->stationCountL->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                              .arg(secondaryColor.red())
                              .arg(secondaryColor.green())
                              .arg(secondaryColor.blue()));
    this->loadThemeIcons();
    this->setButtonIcons();
    this->setLabelText();
    ui->menuList->hide();

    ui->menuList->setItemDelegate(new MainDelegate(ui->menuList));

#ifdef MAFW
    TAGSOURCE_AUDIO_PATH = "localtagfs::music/songs";
    TAGSOURCE_PLAYLISTS_PATH = "localtagfs::music/playlists";
    TAGSOURCE_VIDEO_PATH = "localtagfs::videos";
    RADIOSOURCE_PATH = "iradiosource::";

    mafwFactory = new MafwAdapterFactory(this);
    mafwrenderer = mafwFactory->getRenderer();
    mafwTrackerSource = mafwFactory->getTrackerSource();
    mafwRadioSource = mafwFactory->getRadioSource();
    playlist = mafwFactory->getPlaylistAdapter();
    if (mafwrenderer->isRendererReady())
        mafwrenderer->getStatus();
    else
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);

    ui->songCountL->clear();
    ui->videoCountL->clear();
    ui->stationCountL->clear();

    wiredHeadsetIsConnected = false;
    headsetPauseStamp = -1;
    pausedByCall = false;
    wasRinging = false;
#else
    // Menu bar breaks layouts on desktop, hide it.
    ui->menuBar->hide();
#endif

#ifdef MAFW
    musicWindow = new MusicWindow(this, mafwFactory);
    upnpControl = new UpnpControl(ui->centralWidget, mafwFactory);
    connect(upnpControl, SIGNAL(childOpened()), this, SLOT(onChildOpened()));
    connect(upnpControl, SIGNAL(childClosed()), this, SLOT(onChildClosed()));
#else
    musicWindow = new MusicWindow(this);
#endif

    sleeperTimeoutStamp = -1;
    sleeperTimer = new QTimer(this);
    sleeperTimer->setSingleShot(true);
    sleeperVolumeTimer = new QTimer(this);
    sleeperVolumeTimer->setSingleShot(true);

    this->connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    rotator->setSlave(this);
    reloadSettings();

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif

#ifdef Q_WS_MAEMO_5
    if (mafwTrackerSource->isReady())
        registerDbusService();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(registerDbusService()));

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

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    updatingInfoBox = new QMaemo5InformationBox(this);
    updatingInfoBox->setTimeout(0);
    updatingInfoBox->setWidget(widget);

    QTimer::singleShot(1000, this, SLOT(takeScreenshot()));
    this->updateWiredHeadset();
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

#ifdef Q_WS_MAEMO_5
void MainWindow::registerDbusService()
{
    if (!QDBusConnection::sessionBus().registerService(DBUS_SERVICE)) {
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }

    if (!QDBusConnection::sessionBus().registerObject(DBUS_PATH, this, QDBusConnection::ExportScriptableSlots)) {
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }

    if (!QDBusConnection::sessionBus().registerObject("/com/nokia/mediaplayer", this, QDBusConnection::ExportScriptableSlots)) {
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }
}
#endif

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage(backgroundImage));
}

void MainWindow::loadThemeIcons()
{
    if ( QFileInfo("/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_album.png").exists() )
        albumImage = "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_album.png";
    else
        albumImage = "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_stream.png").exists() )
        radioImage = "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_stream.png";
    else
        radioImage = "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_stream.png";
}

void MainWindow::setButtonIcons()
{
    ui->songsButton->setIcon(QIcon::fromTheme(musicIcon));
    ui->videosButton->setIcon(QIcon::fromTheme(videosIcon));
    ui->radioButton->setIcon(QIcon::fromTheme(radioIcon));
    ui->shuffleAllButton->setIcon(QIcon::fromTheme(shuffleIcon));

    ui->menuList->item(0)->setIcon(QIcon::fromTheme(musicIcon));
    ui->menuList->item(1)->setIcon(QIcon::fromTheme(videosIcon));
    ui->menuList->item(2)->setIcon(QIcon::fromTheme(radioIcon));
    ui->menuList->item(3)->setIcon(QIcon::fromTheme(shuffleIcon));
}

void MainWindow::setLabelText()
{
    ui->songsButtonLabel->setText(tr("Music"));
    ui->videosButtonLabel->setText(tr("Videos"));
    ui->radioButtonLabel->setText(tr("Internet Radio"));
    ui->shuffleLabel->setText(tr("Shuffle all songs"));

    ui->menuList->item(0)->setText(tr("Music"));
    ui->menuList->item(1)->setText(tr("Videos"));
    ui->menuList->item(2)->setText(tr("Internet Radio"));
    ui->menuList->item(3)->setText(tr("Shuffle all songs"));
}

void MainWindow::connectSignals()
{
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), ui->menuOptions), SIGNAL(activated()), ui->menuOptions, SLOT(close()));

    connect(new QShortcut(QKeySequence(Qt::Key_Q), this), SIGNAL(activated()), this, SLOT(showMusicWindow()));
    connect(new QShortcut(QKeySequence(Qt::Key_W), this), SIGNAL(activated()), this, SLOT(showVideosWindow()));
    connect(new QShortcut(QKeySequence(Qt::Key_E), this), SIGNAL(activated()), this, SLOT(showInternetRadioWindow()));
    connect(new QShortcut(QKeySequence(Qt::Key_R), this), SIGNAL(activated()), this, SLOT(onShuffleAllClicked()));

    connect(ui->songsButton, SIGNAL(clicked()), this, SLOT(showMusicWindow()));
    connect(ui->videosButton, SIGNAL(clicked()), this, SLOT(showVideosWindow()));
    connect(ui->radioButton, SIGNAL(clicked()), this, SLOT(showInternetRadioWindow()));
    connect(ui->shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleAllClicked()));

    connect(ui->menuList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));

    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->actionSleeper, SIGNAL(triggered()), this, SLOT(openSleeperDialog()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(musicWindow, SIGNAL(hidden()), this, SLOT(onChildClosed()));

    connect(sleeperTimer, SIGNAL(timeout()), this, SLOT(onSleeperTimeout()));
    connect(sleeperVolumeTimer, SIGNAL(timeout()), this, SLOT(stepSleeperVolume()));

#ifdef MAFW
    connect(mafwRadioSource, SIGNAL(sourceReady()), this, SLOT(radioSourceReady()));
    connect(mafwRadioSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwRadioSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countRadioResult(QString, GHashTable*, QString)));

    connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(trackerSourceReady()));
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(updating(int,int,int,int)), this, SLOT(onSourceUpdating(int,int,int,int)));
    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countAudioVideoResult(QString, GHashTable*, QString)));

    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onPropertyChanged(const QDBusMessage &)));
#endif
#ifdef Q_WS_MAEMO_5
    QDBusConnection::systemBus().connect("", "", "org.bluez.AudioSink", "Connected",
                                         this, SLOT(onHeadsetConnected()));
    QDBusConnection::systemBus().connect("", "", "org.bluez.AudioSink", "Disconnected",
                                         this, SLOT(onHeadsetDisconnected()));
    QDBusConnection::systemBus().connect("", "", "org.bluez.Headset", "Connected",
                                         this, SLOT(onHeadsetConnected()));
    QDBusConnection::systemBus().connect("", "", "org.bluez.Headset", "Disconnected",
                                         this, SLOT(onHeadsetDisconnected()));

    QDBusConnection::systemBus().connect("", "/org/freedesktop/Hal/devices/platform_headphone", "org.freedesktop.Hal.Device", "PropertyModified",
                                         this, SLOT(updateWiredHeadset()));

    QDBusConnection::systemBus().connect("", "", "org.freedesktop.Hal.Device", "Condition",
                                         this, SLOT(onHeadsetButtonPressed(QDBusMessage)));

    QDBusConnection::systemBus().connect("", "", "com.nokia.mce.signal", "sig_call_state_ind",
                                         this, SLOT(onCallStateChanged(QDBusMessage)));
#endif
}

void MainWindow::open_mp_main_view()
{
    closeChildren();
}

void MainWindow::open_mp_now_playing()
{
    closeChildren();

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

    RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this, mafwFactory);
    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    window->show();
    ui->indicator->inhibit();
}

void MainWindow::open_mp_now_playing_playback_on()
{
    this->activateWindow();
}

void MainWindow::open_mp_radio_playing_playback_on()
{
    this->activateWindow();
}

void MainWindow::open_mp_car_view()
{
    closeChildren();

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

    songAddBufferSize = entries.size();
    songAddBuffer = new gchar*[songAddBufferSize+1];
    int songAddBufferPos = 0;

    for (int i = 0; i < songAddBufferSize; i++) {
        QString file = entries.at(i);
        QString mime = gnome_vfs_get_mime_type_for_name(file.toUtf8());

        // maybe there's a better way to ignore playlists
        if (mime.startsWith(filter) && !mime.endsWith("mpegurl"))
            songAddBuffer[songAddBufferPos++] = qstrdup(file.prepend(path)
                                                            .replace(" ", "%20")
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
    QString uriToPlay = uriString;

    // Create an object ID from the provided URI
    if (uriToPlay.startsWith("/")) uriToPlay.prepend("file://");
    QString objectId = mafwTrackerSource->createObjectId(uriToPlay);
    qDebug() << "ID to open:" << objectId;

    // Determine the MIME from the name
    const gchar* text_uri = uriToPlay.toUtf8();
    const char* mimetype = gnome_vfs_get_mime_type_for_name(text_uri);
    QString qmimetype = mimetype;
    qDebug() << "MIME:" << mimetype;

    // Audio or a playlist
    if (qmimetype.startsWith("audio")) {

        // M3U playlist, browse contents
        if (qmimetype.endsWith("mpegurl")) {
#ifdef Q_WS_MAEMO_5
            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
            // Converting urisource object to localtagfs:
            // "urisource::file:///home/user/MyDocs/mix.m3u"
            // "localtagfs::music/playlists/%2Fhome%2Fuser%2FMyDocs%2Fmix.m3u"
            objectId = objectId.remove("urisource::file://")
                               .replace("/", "%2F")
                               .prepend(TAGSOURCE_PLAYLISTS_PATH + QString("/"));
            qDebug() << "Converted ID:" << objectId;

            songAddBufferSize = 0;

            connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                    this, SLOT(browseSongs(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

            // for some reason, if metadata fetching is disabled here, IDs for filesystem instead of localtagfs are returned
            browseSongsId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                            MAFW_SOURCE_LIST (MAFW_METADATA_KEY_MIME),
                                                            0, MAFW_SOURCE_BROWSE_ALL);
        }

        // Audio, a whole directory has to be added
        else if (QSettings().value("main/openFolders").toBool()) {
#ifdef Q_WS_MAEMO_5
            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
            objectIdToPlay = objectId.remove("urisource::file://")
                                     .replace("/", "%2F")
                                     .prepend(TAGSOURCE_AUDIO_PATH + QString("/"));
            qDebug() << "Converted ID:" << objectIdToPlay;

            openDirectory(uriToPlay, Media::Audio);
        }

        // Audio, just one file
        else {
            objectId = objectId.remove("urisource::file://")
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

    // Video only
    } else if (qmimetype.startsWith("video")) {

        // A whole directory has to be added
        if (QSettings().value("main/openFolders").toBool()) {
#ifdef Q_WS_MAEMO_5
            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
            objectIdToPlay = objectId.remove("urisource::file://")
                                     .replace("/", "%2F")
                                     .prepend(TAGSOURCE_VIDEO_PATH + QString("/"));
            qDebug() << "Converted ID:" << objectIdToPlay;

            openDirectory(uriToPlay, Media::Video);
        }

        // Just one file
        else {
            objectId = objectId.remove("urisource::file://")
                               .replace("/", "%2F")
                               .prepend(TAGSOURCE_VIDEO_PATH + QString("/"));
            qDebug() << "Converted ID:" << objectId;

            playlist->assignVideoPlaylist();
            playlist->clear();
            playlist->appendItem(objectId);
            createVideoNowPlayingWindow();
        }
    }

    // Unknown, use the video window for detection
    else if (qmimetype == "application/octet-stream") {
        playlist->assignVideoPlaylist();
        playlist->clear();
        playlist->appendItem(objectId);
        createVideoNowPlayingWindow();
    }
#endif
}

NowPlayingWindow* MainWindow::createNowPlayingWindow()
{
    closeChildren();

#ifdef MAFW
    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
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
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory);
#else
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
    window->showFullScreen();

    connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(restore()));
    ui->indicator->inhibit();

#ifdef MAFW
    QTimer::singleShot(500, window, SLOT(playVideo()));
#endif
}

void MainWindow::showWindowMenu()
{
    ui->menuOptions->adjustSize();
    int x = (this->width() - ui->menuOptions->width()) / 2;
    ui->menuOptions->exec(this->mapToGlobal(QPoint(x,-35)));
}

void MainWindow::orientationChanged(int w, int h)
{
    if (w > h) { // Landscape
        ui->menuList->hide();
        ui->songsButton->show();
        ui->songsButtonLabel->show();
        ui->videosButton->show();
        ui->videosButtonLabel->show();
        ui->radioButton->show();
        ui->radioButtonLabel->show();
        ui->shuffleAllButton->show();
        ui->shuffleLabel->show();
        ui->songCountL->show();
        ui->videoCountL->show();
        ui->stationCountL->show();
    } else { // Portrait
        ui->songsButton->hide();
        ui->songsButtonLabel->hide();
        ui->videosButton->hide();
        ui->videosButtonLabel->hide();
        ui->radioButton->hide();
        ui->radioButtonLabel->hide();
        ui->shuffleAllButton->hide();
        ui->shuffleLabel->hide();
        ui->songCountL->hide();
        ui->videoCountL->hide();
        ui->stationCountL->hide();
        ui->menuList->setGeometry(QRect(0, 0, w, h));
        ui->menuList->show();
    }

    upnpControl->setGeometry(0, h-(70+56), w-(112+8), upnpControl->height());
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);

    upnpControl->raise();
    ui->indicator->raise();
}

void MainWindow::showAbout()
{
    AboutWindow *window = new AboutWindow(this);
    window->show();
}

void MainWindow::processListClicks(QListWidgetItem* item)
{
    QString itemName = item->statusTip();
    if (itemName == "songs_button")
        showMusicWindow();
    else if (itemName == "videos_button")
        showVideosWindow();
    else if (itemName == "radio_button")
        showInternetRadioWindow();
    else if (itemName == "shuffle_button")
        onShuffleAllClicked();
}

void MainWindow::openSettings()
{
    SettingsDialog *settings = new SettingsDialog(this);
    connect(settings, SIGNAL(destroyed()), this, SLOT(reloadSettings()));
    settings->show();
}

void MainWindow::reloadSettings()
{
    NowPlayingWindow::destroy();

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
    SleeperDialog *sleeperDialog = new SleeperDialog(this);
    connect(sleeperDialog, SIGNAL(timerRequested(int,int)), this, SLOT(setSleeperTimer(int,int)));
    connect(this, SIGNAL(sleeperSet(qint64)), sleeperDialog, SLOT(setTimeoutStamp(qint64)));
    sleeperDialog->setTimeoutStamp(sleeperTimeoutStamp);
    sleeperDialog->show();
}

void MainWindow::setSleeperTimer(int interval, int reduction)
{
    sleeperTimer->stop();
    sleeperVolumeTimer->stop();
    volumeReduction = reduction;

    if (interval >= 0) {
        qDebug() << "Setting sleeper timer to" << interval << "ms";

        sleeperTimer->setInterval(interval);
        sleeperTimer->start();
        sleeperStartStamp = QDateTime::currentMSecsSinceEpoch();
        sleeperTimeoutStamp = sleeperStartStamp + interval;

#ifdef MAFW
        connect(mafwrenderer, SIGNAL(signalGetVolume(int)), this, SLOT(getInitialVolume(int)));
        mafwrenderer->getVolume();
#endif
    } else {
        qDebug() << "Aborting sleeper";
        sleeperTimeoutStamp = -1;
    }

    emit sleeperSet(sleeperTimeoutStamp);
}

#ifdef MAFW
void MainWindow::getInitialVolume(int volume)
{
    disconnect(mafwrenderer, SIGNAL(signalGetVolume(int)), this, SLOT(getInitialVolume(int)));
    this->volume = volume;
    scheduleSleeperVolume();
}
#endif

#ifdef MAFW
void MainWindow::onPropertyChanged(const QDBusMessage &msg)
{
    if (msg.arguments()[0].toString() == "volume") {
        volume = qdbus_cast<QVariant>(msg.arguments()[1]).toInt();
        scheduleSleeperVolume();
    }
}
#endif

void MainWindow::scheduleSleeperVolume()
{
    if (volumeReduction && volume > 0) {
        qint64 timespan = sleeperTimeoutStamp - QDateTime::currentMSecsSinceEpoch();
        if (timespan > 0) {
            switch (volumeReduction) {
                case SleeperDialog::LinearReduction:
                    sleeperVolumeTimer->setInterval(timespan / volume);
                    break;
                case SleeperDialog::ExponentialReduction:
                    // The following algorithm is used to determine the timer interval:
                    //     1. Calculate the reference volume for the current moment.
                    //     2. Calculate the scale between the current volume and the reference volume from step 1.
                    //     3. Calculate the reference volume for the current volume minus 1 using the scale from step 2.
                    //     4. Calculate the moment for which the reference volume from step 1 would be equal to the reference volume from step 3.
                    //     5. Calculate the interval as the difference between the moment from step 4 and the current moment.

                    // Exponentially decreasing reference volume can be calculated using the follwing formula:
                    //     v(t) = 100 - (exp(a*t)-1)
                    // Parameter a is constant and adjusts the slope.
                    // Parameter t is the moment for which the reference volume should be calculated.
                    // MAFW accepts volume levels between 0 and 100, so t should be between 0 and ln(100 + 1) / a.

                    const int a = 5;
                    const double tMax = 0.92302410336825;

                    qint64 currentStamp = QDateTime::currentMSecsSinceEpoch();
                    double t = tMax * (currentStamp-sleeperStartStamp) / (sleeperTimeoutStamp-sleeperStartStamp);
                    double referenceVolume = 101 - qExp(a*t);
                    double scale = referenceVolume / volume;
                    double nextReferenceVolume = (volume-1) * scale;
                    double tNext = qLn(101-nextReferenceVolume) / a;
                    int interval = sleeperStartStamp + (sleeperTimeoutStamp-sleeperStartStamp)*(tNext/tMax) - currentStamp;
                    sleeperVolumeTimer->setInterval(qMax(0, interval));
                    break;
            }
            sleeperVolumeTimer->start();
            qDebug() << "Current volume level is" << volume << "and next step is in" << sleeperVolumeTimer->interval() << "ms";
        }
    }
}

void MainWindow::stepSleeperVolume()
{
#ifdef MAFW
    volume = qMax(0, volume-1);
    mafwrenderer->setVolume(volume);
#endif
}

void MainWindow::onSleeperTimeout()
{
    emit sleeperSet(sleeperTimeoutStamp = -1);

    QString action = QSettings().value("timer/action", "stop-playback").toString();
    qDebug() << "Sleeper countdown finished with action" << action;

#ifdef MAFW
    if (action == "stop-playback")
        mafwrenderer->stop();
    else if (action == "pause-playback")
        mafwrenderer->pause();
    else if (action == "close-application")
        this->close();
#endif
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
    VideosWindow *window = new VideosWindow(this, mafwFactory);
#else
    VideosWindow *window = new VideosWindow(this);
#endif
    window->setAttribute(Qt::WA_DeleteOnClose);

    window->show();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}

void MainWindow::showInternetRadioWindow()
{
    this->setEnabled(false);

#ifdef MAFW
    InternetRadioWindow *window = new InternetRadioWindow(this, mafwFactory);
#else
    InternetRadioWindow *window = new InternetRadioWindow(this);
#endif

    window->show();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}

#ifdef MAFW
void MainWindow::trackerSourceReady()
{
    this->countSongs();
    this->countVideos();
}

void MainWindow::radioSourceReady()
{
    countRadioStations();
}

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

        ui->songCountL->setText(countStr);
        ui->menuList->item(0)->setData(UserRoleValueText, countStr);

    } else if (objectId == TAGSOURCE_VIDEO_PATH) {
        QString countStr = (count == -1) ? tr("(no videos)") :
                                           tr("%n clip(s)", "", count);

        ui->videoCountL->setText(countStr);
        ui->menuList->item(1)->setData(UserRoleValueText, countStr);
    }

    if (!error.isEmpty())
        qDebug() << error;
}

void MainWindow::countRadioResult(QString, GHashTable* metadata, QString error)
{
    GValue *v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
    int count = v ? g_value_get_int(v) : -1;

    QString countStr = (count == -1) ? tr("(no stations)") :
                                       tr("%n station(s)", "", count);

    ui->stationCountL->setText(countStr);
    ui->menuList->item(2)->setData(UserRoleValueText, countStr);

    if (!error.isEmpty())
        qDebug() << error;
}

void MainWindow::browseSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata , QString)
{
    if (browseId != browseSongsId) return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;

        QString mime = QString::fromUtf8(g_value_get_string (mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME)));
        qDebug() << "First item MIME:" << mime;

        if (mime.startsWith("audio"))
            playlist->assignAudioPlaylist();
        else
            playlist->assignVideoPlaylist();

        playlist->clear();
    }

    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseSongs(uint,int,uint,QString,GHashTable*,QString)));

        playlist->appendItems((const gchar**)songAddBuffer);

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

        QString mime = QString::fromUtf8(g_value_get_string(mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME)));
        qDebug() << "Last item MIME:" << mime;

        // Assume that the playlist consists of items of one type
        if (mime.startsWith("audio")) {
            mafwrenderer->play();
            createNowPlayingWindow();
        } else {
            createVideoNowPlayingWindow();
        }

#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
        songAddBufferSize = 0;
    }
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

    songAddBufferSize = 0;

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseSongs(uint, int, uint, QString, GHashTable*, QString)), Qt::UniqueConnection);

    browseSongsId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, NULL,
                                                    MAFW_SOURCE_LIST (MAFW_METADATA_KEY_MIME),
                                                    0, MAFW_SOURCE_BROWSE_ALL);
#endif
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QString action = QSettings().value("main/onApplicationExit", "stop-playback").toString();
#ifdef MAFW
    mafwrenderer->enablePlayback(false);
    if (action == "stop-playback")
        mafwrenderer->stop();
    else if (action == "pause-playback")
        mafwrenderer->pause();
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
    text.append(tr("Estimated time remaining:") + " " + (remaining_time < 0 ? "?" : time_mmss(remaining_time)));
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
        mafwrenderer->enablePlayback(QSettings().value("main/managedPlayback") == "locked");
    else
        mafwrenderer->enablePlayback(QSettings().value("main/managedPlayback") == "unlocked");
#endif
}
#endif

#ifdef MAFW
void MainWindow::setupPlayback()
{
    disconnect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));

    QString playback = QSettings().value("main/managedPlayback", "always").toString();
    if (playback == "always")
        mafwrenderer->enablePlayback(true);
    else if (playback == "never")
        mafwrenderer->enablePlayback(false);
    else if (playback == "locked" || playback == "unlocked") {
        connect(Maemo5DeviceEvents::acquire(), SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));
        onScreenLocked(Maemo5DeviceEvents::acquire()->isScreenLocked());
    }
}
#endif

void MainWindow::onGetStatus(MafwPlaylist*,uint,MafwPlayState state,const char*,QString)
{
    this->mafwState = state;
}

void MainWindow::pausePlay()
{
    if (this->mafwState == Playing)
        mafwrenderer->pause();
    else if (this->mafwState == Paused)
        mafwrenderer->resume();
    else if (this->mafwState == Stopped)
        mafwrenderer->play();
}

void MainWindow::onStateChanged(int state)
{
    this->mafwState = state;

    if (state == Playing) {
        headsetPauseStamp = -1;
        pausedByCall = false;
    }
}

void MainWindow::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::music")
        this->countSongs();
    else if (objectId == "localtagfs::videos")
        this->countVideos();
    else if (objectId == "iradiosource::")
        this->countRadioStations();
}
#endif

#ifdef Q_WS_MAEMO_5
void MainWindow::phoneButton()
{
    QString action = QSettings().value("main/headsetButtonAction", "next").toString();
    if (action == "next") {
        if (this->mafwState == Playing)
            mafwrenderer->next();
        else
            this->pausePlay();
    } else if (action == "previous")
        mafwrenderer->previous();
    else if (action == "playpause")
        this->pausePlay();
    else if (action == "stop")
        mafwrenderer->stop();
}

void MainWindow::onHeadsetConnected()
{
    qint64 headsetResumeTime = QSettings().value("main/headsetResumeSeconds", -1).toInt();

    if (headsetPauseStamp != -1
    && mafwState == Paused
    && !pausedByCall
    && (headsetResumeTime == -1 || headsetPauseStamp + headsetResumeTime*1000 > QDateTime::currentMSecsSinceEpoch()))
        mafwrenderer->resume();

    headsetPauseStamp = -1;
}

void MainWindow::onHeadsetDisconnected()
{
    if (QSettings().value("main/pauseHeadset", true).toBool()) {
        if (mafwState == Playing) {
            mafwrenderer->pause();
            headsetPauseStamp = QDateTime::currentMSecsSinceEpoch();
        } else if (pausedByCall) {
            headsetPauseStamp = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void MainWindow::updateWiredHeadset()
{
    QDBusInterface interface("org.freedesktop.Hal", HAL_PATH_RX51_JACK, "org.freedesktop.Hal.Device", QDBusConnection::systemBus(), this);
    QDBusInterface interface2("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/platform_headphone", "org.freedesktop.Hal.Device", QDBusConnection::systemBus(), this);

    if (!interface.isValid() || !interface2.isValid())
        return;

    // jackList contains "headphone" when headset is connected
    QStringList jackList = static_cast< QDBusReply <QStringList> >(interface.call("GetProperty", "input.jack.type"));

    // state contains jack GPIO state - false when nothing is not connected to jack
    bool state = static_cast< QDBusReply <bool> >(interface2.call("GetProperty", "button.state.value"));

    // "Jack Bias" is alsa switch, when is off wired headset button is disabled
    QStringList args;
    args << "-c" << "0" << "sset" << "Jack Bias";

    bool connected = ( state && jackList.contains("headphone") );

    if (connected && !wiredHeadsetIsConnected) {
        onHeadsetConnected();
        QProcess::startDetached("amixer", args << "on");
        wiredHeadsetIsConnected = true;
    } else if (!connected && wiredHeadsetIsConnected) {
        onHeadsetDisconnected();
        QProcess::startDetached("amixer", args << "off");
        wiredHeadsetIsConnected = false;
    }
}

void MainWindow::onHeadsetButtonPressed(QDBusMessage msg)
{
    if (msg.arguments()[0] == "ButtonPressed") {
        if (msg.arguments()[1] == "play-cd" || msg.arguments()[1] == "pause-cd")
            this->pausePlay();
        else if (msg.arguments()[1] == "stop-cd")
            mafwrenderer->stop();
        else if (msg.arguments()[1] == "next-song")
            mafwrenderer->next();
        else if (msg.arguments()[1] == "previous-song")
            mafwrenderer->previous();
        else if (msg.arguments()[1] == "fast-forward")
            mafwrenderer->setPosition(SeekRelative, 3);
        else if (msg.arguments()[1] == "rewind")
            mafwrenderer->setPosition(SeekRelative, -3);
        else if (msg.arguments()[1] == "phone")
            this->phoneButton();
        else if (msg.arguments()[1] == "jack_insert" && msg.path() == HAL_PATH_RX51_JACK) // wired headset was connected or disconnected
            this->updateWiredHeadset();
    }
}

void MainWindow::onCallStateChanged(QDBusMessage msg)
{
    QString state = msg.arguments()[0].toString();

    if (state == "ringing") {
        wasRinging = true;
        pausedByCall = mafwState == Playing;
        if (pausedByCall) mafwrenderer->pause();
    }

    else if (!wasRinging && state == "active") {
        pausedByCall = mafwState == Playing;
        if (pausedByCall) mafwrenderer->pause();
    }

    else if (state == "none") {
        if (pausedByCall && headsetPauseStamp == -1)
            mafwrenderer->resume();
        pausedByCall = false;
        wasRinging = false;
    }
}

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
