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
QString musicIcon, videosIcon, radioIcon, shuffleIcon, defaultAlbumArt,
    defaultAlbumArtMedium, defaultVideoImage, volumeButtonIcon, albumImage,
    radioImage, shareButtonIcon, deleteButtonIcon;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->loadThemeIcons();
    this->setButtonIcons();
    this->setLabelText();
    ui->listWidget->hide();
    dbusNowPlaying = 0;

    MainDelegate *delegate = new MainDelegate(ui->listWidget);
    ui->listWidget->setItemDelegate(delegate);

#ifdef MAFW
    TAGSOURCE_AUDIO_PATH = "localtagfs::music/songs";
    TAGSOURCE_VIDEO_PATH = "localtagfs::videos";
    RADIOSOURCE_PATH = "iradiosource::";
    mafwFactory = new MafwAdapterFactory(this);
    mafwrenderer = mafwFactory->getRenderer();
    mafwTrackerSource = mafwFactory->getTrackerSource();
    mafwRadioSource = mafwFactory->getRadioSource();
    playlist = mafwFactory->getPlaylistAdapter();
    this->shuffleNowPlayingWindowCreated = false;
    if (mafwrenderer->isRendererReady())
        mafwrenderer->getStatus();
    else
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
#endif

#ifdef Q_WS_MAEMO_5
    if (!QDBusConnection::sessionBus().registerService(DBUS_SERVICE)) {
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }

    if (!QDBusConnection::sessionBus().registerObject(DBUS_PATH, this, QDBusConnection::ExportScriptableSlots)) {
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }

    if (!QDBusConnection::sessionBus().registerObject("/com/nokia/mediaplayer", this, QDBusConnection::ExportScriptableSlots)) {
        qWarning("%s", qPrintable(QDBusConnection::sessionBus().lastError().message()));
    }

    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);

    ui->songCountL->clear();
    ui->videoCountL->clear();
    ui->startionCountL->clear();

    updatingIndex = 0;
#else
    // Menu bar breaks layouts on desktop, hide it.
    ui->menuBar->hide();
#endif

#ifdef MAFW
    myMusicWindow = new MusicWindow(this, mafwFactory);
#else
    myMusicWindow = new MusicWindow(this);
#endif

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    this->connectSignals();

    this->orientationChanged();

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif
#ifdef Q_WS_MAEMO_5
    QTimer::singleShot(1000, this, SLOT(takeScreenshot()));
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage(backgroundImage));
}

void MainWindow::loadThemeIcons()
{
    if ( QFileInfo("/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_music.png").exists() )
        musicIcon = "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_music.png";
    else
        musicIcon = "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_music.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_video.png").exists() )
        videosIcon = "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_video.png";
    else
        videosIcon = "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_video.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_radio.png").exists() )
        radioIcon = "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_radio.png";
    else
        radioIcon = "/usr/share/icons/hicolo/164x164/hildon/mediaplayer_main_button_radio.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_shuffle.png").exists() )
        shuffleIcon = "/usr/share/icons/"+currtheme+"/164x164/hildon/mediaplayer_main_button_shuffle.png";
    else
        shuffleIcon = "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_shuffle.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/64x64/hildon/mediaplayer_default_album.png").exists() )
        defaultAlbumArt = "/usr/share/icons/"+currtheme+"/64x64/hildon/mediaplayer_default_album.png";
    else
        defaultAlbumArt = "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_default_album.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/124x124/hildon/mediaplayer_default_album.png").exists() )
        defaultAlbumArtMedium = "/usr/share/icons/"+currtheme+"/124x124/hildon/mediaplayer_default_album.png";
    else
        defaultAlbumArtMedium = "/usr/share/icons/hicolor/124x124/hildon/mediaplayer_default_album.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/124x124/hildon/general_video.png").exists() )
        defaultVideoImage = "/usr/share/icons/"+currtheme+"/124x124/hildon/general_video.png";
    else
        defaultVideoImage = "/usr/share/icons/hicolor/124x124/hildon/general_video.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/64x64/hildon/mediaplayer_volume.png").exists() )
        volumeButtonIcon = "/usr/share/icons/"+currtheme+"/64x64/hildon/mediaplayer_volume.png";
    else
        volumeButtonIcon = "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_album.png").exists() )
        albumImage = "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_album.png";
    else
        albumImage = "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_stream.png").exists() )
        radioImage = "/usr/share/icons/"+currtheme+"/295x295/hildon/mediaplayer_default_stream.png";
    else
        radioImage = "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_stream.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/48x48/hildon/general_share.png").exists() )
        shareButtonIcon = "/usr/share/icons/"+currtheme+"/48x48/hildon/general_share.png";
    else
        shareButtonIcon = "/usr/share/icons/hicolor/48x48/hildon/general_share.png";

    if ( QFileInfo("/usr/share/icons/"+currtheme+"/48x48/hildon/general_delete.png").exists() )
        deleteButtonIcon = "/usr/share/icons/"+currtheme+"/48x48/hildon/general_delete.png";
    else
        deleteButtonIcon = "/usr/share/icons/hicolor/48x48/hildon/general_delete.png";
}

void MainWindow::setButtonIcons()
{
    ui->songsButton->setIcon(QIcon(musicIcon));
    ui->videosButton->setIcon(QIcon(videosIcon));
    ui->radioButton->setIcon(QIcon(radioIcon));
    ui->shuffleAllButton->setIcon(QIcon(shuffleIcon));

    ui->listWidget->item(0)->setData(Qt::UserRole+1, musicIcon);
    ui->listWidget->item(1)->setData(Qt::UserRole+1, videosIcon);
    ui->listWidget->item(2)->setData(Qt::UserRole+1, radioIcon);
    ui->listWidget->item(3)->setData(Qt::UserRole+1, shuffleIcon);


}

void MainWindow::setLabelText()
{
    ui->songsButtonLabel->setText(tr("Music"));
    ui->videosButtonLabel->setText(tr("Videos"));
    ui->radioButtonLabel->setText(tr("Internet Radio"));
    ui->shuffleLabel->setText(tr("Shuffle all songs"));

    ui->listWidget->item(0)->setData(Qt::UserRole, tr("Music"));
    ui->listWidget->item(1)->setData(Qt::UserRole, tr("Videos"));
    ui->listWidget->item(2)->setData(Qt::UserRole, tr("Internet Radio"));
    ui->listWidget->item(3)->setData(Qt::UserRole, tr("Shuffle all songs"));
}

void MainWindow::connectSignals()
{
    connect(ui->songsButton, SIGNAL(clicked()), myMusicWindow, SLOT(show()));
    connect(ui->videosButton, SIGNAL(clicked()), this, SLOT(showVideosWindow()));
    connect(ui->radioButton, SIGNAL(clicked()), this, SLOT(showInternetRadioWindow()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));
    // TODO: Connect this to a slot.
    // connect(ui->indicator, SIGNAL(clicked()), this, SLOT();
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(myMusicWindow, SIGNAL(shown()), ui->indicator, SLOT(hide()));
    connect(myMusicWindow, SIGNAL(hidden()), ui->indicator, SLOT(show()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(trackerSourceReady()));
    //connect(mafwTrackerSource, SIGNAL(updating(int,int,int,int)), this, SLOT(onSourceUpdating(int,int,int,int)));
    connect(mafwRadioSource, SIGNAL(sourceReady()), this, SLOT(radioSourceReady()));
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
                                         this, SLOT(onBluetoothButtonPressed(QDBusMessage)));
#endif
}

void MainWindow::open_mp_now_playing()
{
    if (mafwrenderer->isRendererReady() && mafwTrackerSource->isReady() && !playlist->isPlaylistNull()) {
        this->createNowPlayingWindow();
    } else {
        QTimer::singleShot(2000, this, SLOT(createNowPlayingWindow()));
    }
}

void MainWindow::mime_open(const QString &uriString)
{
    this->activateWindow();
    this->uriToPlay = uriString;
    if (uriToPlay.startsWith("/"))
        uriToPlay.prepend("file:/");
    QString objectId = mafwTrackerSource->createObjectId(uriToPlay);
    qDebug() << objectId;
    const gchar* text_uri = uriToPlay.toUtf8();
    const char* mimetype = gnome_vfs_get_mime_type_for_name(text_uri);
    QString qmimetype = mimetype;
    if (qmimetype.startsWith("audio")) {
#ifdef MAFW
        playlist->assignAudioPlaylist();
        playlist->clear();
        playlist->appendItem(objectId);
#endif
        NowPlayingWindow *nowPlayingWindow;
#ifdef MAFW
        nowPlayingWindow = new NowPlayingWindow(this, mafwFactory);
#else
        nowPlayingWindow = new NowPlayingWindow(this);
#endif
        nowPlayingWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(nowPlayingWindow, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        connect(nowPlayingWindow, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
        nowPlayingWindow->show();
    } else if(qmimetype.startsWith("video")) {
#ifdef MAFW
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory);
#else
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
        connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        window->showFullScreen();
#ifdef MAFW
        window->playObject(objectId);
#endif
    }
#ifdef MAFW
    if (mafwrenderer->isRendererReady()) {
        mafwrenderer->play();
        mafwrenderer->resume();
    } else {
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(play()));
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(resume()));
    }
#endif
    ui->indicator->hide();
}

void MainWindow::createNowPlayingWindow()
{
    if (dbusNowPlaying == 0) {
#ifdef MAFW
        dbusNowPlaying = new NowPlayingWindow(this, mafwFactory);
#else
        dbusNowPlaying = new NowPlayingWindow(this);
#endif
        dbusNowPlaying->setAttribute(Qt::WA_DeleteOnClose);
        connect(dbusNowPlaying, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        connect(dbusNowPlaying, SIGNAL(destroyed()), this, SLOT(onDbusNpWindowDestroyed()));
    } else {
        dbusNowPlaying->activateWindow();
    }
    dbusNowPlaying->show();
    ui->indicator->hide();
}

void MainWindow::onDbusNpWindowDestroyed()
{
    dbusNowPlaying = 0;
}

void MainWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()){
        // Landscape mode
#ifdef DEBUG
        qDebug() << "MainWindow: Orientation changed: Landscape.";
#endif
        if(!ui->listWidget->isHidden()) {
            ui->listWidget->hide();
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
        ui->startionCountL->show();
        }
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
        ui->startionCountL->hide();
        ui->listWidget->setGeometry(QRect(0, 0, 480, 800));
        if(ui->listWidget->isHidden())
            ui->listWidget->show();
    }
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55),
                               ui->indicator->width(),ui->indicator->height());
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
    if(itemName == "songs_button")
        myMusicWindow->show();
    else if(itemName == "videos_button")
        this->showVideosWindow();
    else if(itemName == "radio_button")
        this->showInternetRadioWindow();
    else if(itemName == "shuffle_button")
        this->onShuffleAllClicked();
    ui->listWidget->clearSelection();
}

void MainWindow::openSettings()
{
    SettingsDialog *settings = new SettingsDialog(this);
    settings->show();
}

void MainWindow::showVideosWindow()
{
#ifdef MAFW
    myVideosWindow = new VideosWindow(this, mafwFactory);
#else
    myVideosWindow = new VideosWindow(this);
#endif
    myVideosWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(myVideosWindow, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
    myVideosWindow->show();
    ui->indicator->hide();
}

void MainWindow::showInternetRadioWindow()
{
#ifdef MAFW
    myInternetRadioWindow = new InternetRadioWindow(this, mafwFactory);
#else
    myInternetRadioWindow = new InternetRadioWindow(this);
#endif
    myInternetRadioWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(myInternetRadioWindow, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
    myInternetRadioWindow->show();
    ui->indicator->hide();
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
    int count = -1;
    GValue *v;
    v = mafw_metadata_first(metadata,
                            MAFW_METADATA_KEY_CHILDCOUNT_1);
    if(v)
        count = g_value_get_int(v);

    QString countStr;
    countStr.setNum(count);

    if(objectId == TAGSOURCE_AUDIO_PATH) {
        if(count == 1)
            countStr.append(" " + tr("song"));
        else if(count == -1)
            countStr = tr("(no songs)");
        else
            countStr.append(" " + tr("songs"));
        ui->songCountL->setText(countStr);
        ui->listWidget->item(0)->setData(Qt::UserRole+2, countStr);
    } else if(objectId == TAGSOURCE_VIDEO_PATH) {
        if(count == 1)
            countStr.append(" " + tr("clip"));
        else if(count == -1)
            countStr = tr("(no videos)");
        else
            countStr.append(" " + tr("clips"));
        ui->videoCountL->setText(countStr);
        ui->listWidget->item(1)->setData(Qt::UserRole+2, countStr);
    }
    if(!error.isEmpty())
        qDebug() << error;
}

void MainWindow::countRadioResult(QString, GHashTable* metadata, QString error)
{
    int count = -1;
    GValue *v;
    v = mafw_metadata_first(metadata,
                            MAFW_METADATA_KEY_CHILDCOUNT_1);
    if(v)
        count = g_value_get_int(v);

    if(count == 1)
        ui->startionCountL->setText(QString::number(count) + " " + tr("station"));
    else if(count == -1)
        ui->startionCountL->setText(tr("(no stations)"));
    else
        ui->startionCountL->setText(QString::number(count) + " " + tr("stations"));
    ui->listWidget->item(2)->setData(Qt::UserRole+2, ui->startionCountL->text());
    if(!error.isEmpty())
        qDebug() << error;
}

void MainWindow::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* , QString)
{
    if(browseId != browseAllSongsId)
      return;

    playlist->appendItem(objectId);

    if (remainingCount == 0) {
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
        uint randomIndex = qrand() % ((playlist->getSize() + 1) - 0) + 0;
        mafwrenderer->gotoIndex(randomIndex);
        mafwrenderer->play();
        NowPlayingWindow *window = new NowPlayingWindow(this, mafwFactory);
        window->setAttribute(Qt::WA_DeleteOnClose);
        connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        window->show();
        ui->indicator->hide();
        ui->shuffleAllButton->setDisabled(false);
    }
}

#endif

void MainWindow::onShuffleAllClicked()
{
#ifdef MAFW
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(true);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllSongsId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, NULL, 0,
                                                             0, MAFW_SOURCE_BROWSE_ALL);

    ui->shuffleAllButton->setDisabled(true);
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
    if (QSettings().value("main/onApplicationExit").toString() == "stop-playback")
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

#endif
#ifdef Q_WS_MAEMO_5
void MainWindow::onBluetoothButtonPressed(QDBusMessage msg)
{
    if (msg.arguments()[0] == "ButtonPressed") {
        if (msg.arguments()[1] == "play-cd" || msg.arguments()[1] == "pause-cd") {
            this->pausePlay();
        } else if (msg.arguments()[1] == "next-song")
            mafwrenderer->next();
        else if (msg.arguments()[1] == "previous-song")
            mafwrenderer->previous();
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
