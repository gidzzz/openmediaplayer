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
    setAttribute(Qt::WA_Maemo5AutoOrientation);

    ui->songCountL->clear();
    ui->videoCountL->clear();
    ui->stationCountL->clear();

    updatingIndex = 0;

    wasRinging = false;
    wasPlaying = false;
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

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    this->connectSignals();

    this->orientationChanged();

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif

#ifdef Q_WS_MAEMO_5
    if (mafwTrackerSource->isReady())
        registerDbusService();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(registerDbusService()));

    QTimer::singleShot(1000, this, SLOT(takeScreenshot()));
    this->checkPhoneButton();
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
    connect(ui->songsButton, SIGNAL(clicked()), this, SLOT(showMusicWindow()));
    connect(ui->videosButton, SIGNAL(clicked()), this, SLOT(showVideosWindow()));
    connect(ui->radioButton, SIGNAL(clicked()), this, SLOT(showInternetRadioWindow()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->menuList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(musicWindow, SIGNAL(hidden()), this, SLOT(onChildClosed()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(trackerSourceReady()));
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    //connect(mafwTrackerSource, SIGNAL(updating(int,int,int,int)), this, SLOT(onSourceUpdating(int,int,int,int)));
    connect(mafwRadioSource, SIGNAL(sourceReady()), this, SLOT(radioSourceReady()));
    connect(mafwRadioSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countAudioVideoResult(QString, GHashTable*, QString)));
    connect(mafwRadioSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countRadioResult(QString, GHashTable*, QString)));
    connect(ui->shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleAllClicked()));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
#endif
#ifdef Q_WS_MAEMO_5
    QDBusConnection::systemBus().connect("", "", "org.freedesktop.Hal.Device", "Condition",
                                         this, SLOT(onButtonPressed(QDBusMessage)));

    QDBusConnection::systemBus().connect("", "", "com.nokia.mce.signal", "sig_call_state_ind",
                                         this, SLOT(onCallStateChanged(QDBusMessage)));
#endif
}

void MainWindow::open_mp_now_playing()
{
    // maybe this check could be moved to NowPlayingWindow?
    if (mafwrenderer->isRendererReady() && mafwTrackerSource->isReady() && !playlist->isPlaylistNull()) {
        this->createNowPlayingWindow();
    } else {
        QTimer::singleShot(1000, this, SLOT(open_mp_now_playing()));
    }
}

#ifdef MAFW
void MainWindow::openDirectory(QString path) {
    path = path.left(path.lastIndexOf('/'))
               .remove("file://")
               .replace("%20", " ");

    QDir dir(path);
    dir.setFilter(QDir::Files);
    QStringList entries = dir.entryList();

    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(openDirectoryProxy(QString, GHashTable*, QString)));

    songAddBufferSize = entries.size();
    for (int i = 0; i < songAddBufferSize; i++) {
        QString objectId = entries.at(i);
        objectId.prepend(path + "/")
                .replace(" ", "%20")
                .replace("/", "%2F")
                .prepend("localtagfs::music/songs/");
        mafwTrackerSource->getMetadata(objectId.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_MIME));
    }

    songAddBuffer = new gchar*[songAddBufferSize+1];
    songAddBufferPos = 0;
}
#endif

#ifdef MAFW
void MainWindow::openDirectoryProxy(QString objectId, GHashTable *metadata, QString) {
    --songAddBufferSize; // potential collisions with shuffle-all?

    if (metadata) {
        GValue *v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_MIME);
        QString mime = v ? g_value_get_string (v) : "";

        // maybe there's a better way to ignore playlists
        if (mime.startsWith("audio") && mime != "audio/mpegurl"
                                     && mime != "audio/x-mpegurl")
            songAddBuffer[songAddBufferPos++] = qstrdup(objectId.toUtf8());
    }

    if (songAddBufferSize == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
                   this, SLOT(openDirectoryProxy(QString, GHashTable*, QString)));

        songAddBuffer[songAddBufferPos] = NULL;

        if (playlist->playlistName() != "FmpAudioPlaylist")
            playlist->assignAudioPlaylist();
        if (!QSettings().value("main/appendSongs").toBool())
            playlist->clear();

        playlist->appendItems((const gchar**)songAddBuffer);

        if (!QSettings().value("main/appendSongs").toBool()) {
            for (int i = 0; i < songAddBufferPos; i++) {
                if (songAddBuffer[i] == objectIdToPlay) {
                    playlist->getSize(); // explained in musicwindow.cpp (but it seems like it's not enough here?)
                    mafwrenderer->gotoIndex(i);
                    mafwrenderer->play();
                    break;
                }
            }
        }

        createNowPlayingWindow();

        for (int i = 0; i < songAddBufferPos; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    }
}
#endif

void MainWindow::mime_open(const QString &uriString)
{
    //this->activateWindow();
    this->uriToPlay = uriString;
    if (uriToPlay.startsWith("/"))
        uriToPlay.prepend("file://");
    QString objectId = mafwTrackerSource->createObjectId(uriToPlay);
    qDebug() << objectId;
    const gchar* text_uri = uriToPlay.toUtf8();
    const char* mimetype = gnome_vfs_get_mime_type_for_name(text_uri);
    QString qmimetype = mimetype;

    if (qmimetype.startsWith("audio")) {

        if (qmimetype.endsWith("mpegurl")) {
#ifdef MAFW
#ifdef Q_WS_MAEMO_5
            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
            // Converting urisource object to localtagfs:
            // "urisource::file:///home/user/MyDocs/mix.m3u"
            // "localtagfs::music/playlists/%2Fhome%2Fuser%2FMyDocs%2Fmix.m3u"
            objectId = objectId.remove("urisource::file://")
                               .replace("/", "%2F")
                               .prepend(TAGSOURCE_PLAYLISTS_PATH + QString("/"));
            qDebug() << objectId;

            playlist->assignAudioPlaylist();
            playlist->clear();

            songAddBufferSize = 0;

            qDebug() << "connecting MainWindow to signalSourceBrowseResult";
            connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                    this, SLOT(browseSongs(uint,int,uint,QString,GHashTable*,QString)));

            // for some reason, if metadata fetching is disabled here, IDs for filesystem instead of localtagfs are returned
            this->browseSongsId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                                  MAFW_SOURCE_LIST (MAFW_METADATA_KEY_TITLE,
                                                                           MAFW_METADATA_KEY_DURATION,
                                                                           MAFW_METADATA_KEY_ARTIST,
                                                                           MAFW_METADATA_KEY_ALBUM),
                                                                  0, MAFW_SOURCE_BROWSE_ALL);
#endif
        }

        else if (QSettings().value("main/openFolders").toBool()) {
#ifdef MAFW
#ifdef Q_WS_MAEMO_5
            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
            objectIdToPlay = objectId.remove("urisource::file://")
                                     .replace("/", "%2F")
                                     .prepend(TAGSOURCE_AUDIO_PATH + QString("/"));
            openDirectory(uriString);
#endif
        }

        else {
#ifdef MAFW
            objectId = objectId.remove("urisource::file://")
                               .replace("/", "%2F")
                               .prepend(TAGSOURCE_AUDIO_PATH + QString("/"));
            qDebug() << objectId;

            if (playlist->playlistName() != "FmpAudioPlaylist")
                playlist->assignAudioPlaylist();
            if (!QSettings().value("main/appendSongs").toBool())
                playlist->clear();
            playlist->appendItem(objectId);
#endif

            if (!QSettings().value("main/appendSongs").toBool()) {
#ifdef MAFW
                if (mafwrenderer->isRendererReady()) mafwrenderer->play();
                else connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(play()));
#endif
            }

            createNowPlayingWindow();

        }

    } else if (qmimetype.startsWith("video")) {
#ifdef MAFW
        objectId = objectId.remove("urisource::file://")
                           .replace("/", "%2F")
                           .prepend(TAGSOURCE_VIDEO_PATH + QString("/"));
        qDebug() << objectId;

        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory, mafwTrackerSource);
#else
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
        window->showFullScreen();

        connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(restore()));
        ui->indicator->inhibit();
#ifdef MAFW
        playlist->assignVideoPlaylist();
        playlist->clear();
        playlist->appendItem(objectId);
        QTimer::singleShot(500, window, SLOT(playVideo()));
#endif
    }

    else if (qmimetype == "application/octet-stream") {
#ifdef MAFW
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory);
#else
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
        window->showFullScreen();

        connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(restore()));
        ui->indicator->inhibit();
#ifdef MAFW
        playlist->assignVideoPlaylist();
        playlist->clear();
        playlist->appendItem(objectId);
        QTimer::singleShot(500, window, SLOT(playVideo()));
#endif
    }
}

void MainWindow::createNowPlayingWindow()
{
    QList<QMainWindow*> windows = findChildren<QMainWindow*>();
    for (int i = 0; i < windows.size(); i++)
        windows.at(i)->close();

#ifdef MAFW
    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
#else
    NowPlayingWindow *window = NowPlayingWindow::acquire(this);
#endif

    window->show();
    //window->activateWindow();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();
}

void MainWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()){
        // Landscape mode
#ifdef DEBUG
        qDebug() << "MainWindow: Orientation changed: Landscape.";
#endif
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
    } else {
        // Portrait mode
#ifdef DEBUG
        qDebug() << "MainWindow: Orientation changed: Portrait.";
#endif
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
        ui->menuList->setGeometry(QRect(0, 0, 480, 800));
        ui->menuList->show();
    }
    upnpControl->setGeometry(0, screenGeometry.height()-(70+55),
                             screenGeometry.width()-122, upnpControl->height());
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55),
                               ui->indicator->width(), ui->indicator->height());
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
        this->showMusicWindow();
    else if (itemName == "videos_button")
        this->showVideosWindow();
    else if (itemName == "radio_button")
        this->showInternetRadioWindow();
    else if (itemName == "shuffle_button")
        this->onShuffleAllClicked();
}

void MainWindow::openSettings()
{
    SettingsDialog *settings = new SettingsDialog(this);
    settings->show();
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
    window->setAttribute(Qt::WA_DeleteOnClose);

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

void MainWindow::browseSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* , QString)
{
    if (browseId != browseSongsId) return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        qDebug() << "disconnecting MainWindow from signalSourceBrowseResult";
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                   this, SLOT(browseSongs(uint, int, uint, QString, GHashTable*, QString)));

        playlist->appendItems((const gchar**)songAddBuffer);

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;
        this->browseSongsId = 0;

        playlist->getSize(); // explained in musicwindow.cpp
        mafwrenderer->play();

        createNowPlayingWindow();

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

    qDebug() << "connecting MainWindow to signalSourceBrowseResult";
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseSongs(uint, int, uint, QString, GHashTable*, QString)));

    this->browseSongsId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, NULL, 0,
                                                          0, MAFW_SOURCE_BROWSE_ALL);
#endif
}

void MainWindow::focusInEvent(QFocusEvent *)
{
    qDebug() << "MainWindow: focused.";
    ui->indicator->triggerAnimation();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        qDebug() << "Window activated";
        ui->indicator->triggerAnimation();
    }
    else if (event->type() == QEvent::WindowDeactivate) {
        qDebug() << "Window deactivated";
        ui->indicator->stopAnimation();
    }
}

void MainWindow::focusOutEvent(QFocusEvent *)
{
    qDebug() << "MainWindow: focus lost";
    ui->indicator->stopAnimation();
}

void MainWindow::closeEvent(QCloseEvent *)
{
#ifdef MAFW
    if (QSettings().value("main/onApplicationExit").toString() != "do-nothing")
        mafwrenderer->stop();
#endif
}

#ifdef MAFW
void MainWindow::onSourceUpdating(int progress, int processed_items, int remaining_items, int remaining_time)
{
    if (remaining_time != -1 && remaining_time != 0) {
        QTime t(0, 0);
        t = t.addSecs(remaining_time);
        QString text = QString("\nRetrieving information on the new media files.\nEstimated time remaining: %1\n").arg(t.toString("mm:ss"));
        if (processed_items != -1)
            text.append(tr("Processed items:") + " " + QString::number(processed_items).replace("\"",""));
        if (processed_items != -1 && remaining_items != -1)
            text.append("\n");
        if (remaining_items != -1)
            text.append(tr("Remaining items:") + " " + QString::number(remaining_items).replace("\"",""));
#ifdef Q_WS_MAEMO_5
        if (updatingIndex == 0) {
            updatingIndex = new QMaemo5InformationBox(this);
            QWidget *widget = new QWidget(updatingIndex);
            updatingIndex->setTimeout(QMaemo5InformationBox::NoTimeout);
            updatingIndex->setMinimumHeight(140);
            updatingLabel = new QLabel(updatingIndex);
            updatingLabel->setText(text);
            updatingProgressBar = new QProgressBar(updatingIndex);
            updatingProgressBar->setValue(progress);
            updatingIndex->setWidget(widget);

            QVBoxLayout *layout = new QVBoxLayout(widget);
            layout->addWidget(updatingLabel);
            layout->addWidget(updatingProgressBar);
            updatingIndex->exec();
        } else {
            updatingLabel->setText(text);
            updatingProgressBar->setValue(progress);
        }
#endif
    }
}

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
    QString action = QSettings().value("main/headsetButtonAction").toString();
    if (action.isEmpty() || action == "next") {
        if (this->mafwState == Playing)
            mafwrenderer->next();
        else
            this->pausePlay();
    } else if (action == "previous")
        mafwrenderer->previous();
    else if (action == "play")
        this->pausePlay();
    else if (action == "stop")
        mafwrenderer->stop();
}

void MainWindow::checkPhoneButton()
{
    QDBusInterface interface("org.freedesktop.Hal", "/org/freedesktop/Hal/devices/platform_soc_audio_logicaldev_input", "org.freedesktop.Hal.Device", QDBusConnection::systemBus());
    if (!interface.isValid())
        return;

    QStringList jackList = static_cast< QDBusReply <QStringList> >(interface.call("GetProperty", "input.jack.type"));

    QStringList args;
    args << "-c" << "0" << "sset" << "Jack Bias";

    if (jackList.isEmpty())
        QProcess::execute("amixer", args << "off");
    else
        QProcess::execute("amixer", args << "on");
}

void MainWindow::onButtonPressed(QDBusMessage msg)
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
        else if (msg.arguments()[1] == "jack_insert")
            this->checkPhoneButton();
    }
}

void MainWindow::onCallStateChanged(QDBusMessage msg)
{
    QString state = msg.arguments()[0].toString();

    if (state == "ringing") {
        wasRinging = true;
        wasPlaying = mafwState == Playing;
        if (wasPlaying) mafwrenderer->pause();
    }

    else if (!wasRinging && state == "active") {
        wasPlaying = mafwState == Playing;
        if (wasPlaying) mafwrenderer->pause();
    }

    else if (state == "none") {
        wasRinging = false;
        if (wasPlaying) mafwrenderer->resume();
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
