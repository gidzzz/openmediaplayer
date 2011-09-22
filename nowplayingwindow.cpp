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

const int BATCH_SIZE = 100;

NowPlayingWindow* NowPlayingWindow::instance = NULL;
bool newSong;
QString currentPlaylistUrl;

NowPlayingWindow* NowPlayingWindow::acquire(QWidget *parent, MafwAdapterFactory *mafwFactory)
{
    if (instance) {
        instance->setParent(parent, Qt::Window);
        qDebug() << "handing out running NPW instance";
    }
    else {
        qDebug() << "handing out new NPW instance";
        instance = new NowPlayingWindow(parent, mafwFactory);
    }

    return instance;
}

void NowPlayingWindow::destroy()
{
    if (instance) {
        delete instance;
        instance = NULL;
    }
}

NowPlayingWindow::NowPlayingWindow(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
#ifdef MAFW
    ui(new Ui::NowPlayingWindow),
    mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#else
    ui(new Ui::NowPlayingWindow)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif
    setAttribute(Qt::WA_DeleteOnClose);
    positionTimer = new QTimer(this);
    positionTimer->setInterval(1000);

    browseId = NULL;
    newSong = true;
    albumArtScene = new QGraphicsScene(ui->view);
    entertainmentView = 0;
    carView = 0;
    enableLyrics = QSettings().value("lyrics/enable").toBool();

    ui->volSliderWidget->hide();
    ui->lyrics->hide();

    currentPlaylistUrl = "";
    QDir dir ( "/home/user/.mafw-playlists/", "*" );
    dir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot );
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        if ( currentPlaylistUrl == "" )
        {
            QFile data("/home/user/.mafw-playlists/"+QFileInfo(list.at(i)).baseName());
            if (data.open(QFile::ReadOnly | QFile::Truncate))
            {
                QTextStream out(&data);
                while ( !out.atEnd() && currentPlaylistUrl=="")
                {
                    QString line = out.readLine();
                    if ( line.contains(playlist->playlistName()) )
                    {
                        currentPlaylistUrl = "/home/user/.mafw-playlists/"+QFileInfo(list.at(i)).baseName();
                        break;
                    }
                }
            }
            data.close();
        }
    }
    //qDebug() << "Current playlist: " << currentPlaylistUrl;


    this->playlistRequested = false;

    PlayListDelegate *delegate = new PlayListDelegate(ui->songPlaylist);
    ui->songPlaylist->setItemDelegate(delegate);

    ui->songPlaylist->setContextMenuPolicy(Qt::CustomContextMenu);

    this->setButtonIcons();
    //ui->buttonsWidget->setLayout(ui->buttonsLayout);
    ui->songPlaylist->hide();
    QMainWindow::setCentralWidget(ui->verticalWidget);

    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);

    buttonWasDown = false;

#ifdef Q_WS_MAEMO_5
    lastPlayingSong = new GConfItem("/apps/mediaplayer/last_playing_song", this);

    deviceEvents = new Maemo5DeviceEvents(this);
#endif

    this->stateChanged(mafwFactory->mafwState());

    this->connectSignals();


    ui->shuffleButton->setFixedSize(ui->shuffleButton->sizeHint());
    ui->repeatButton->setFixedSize(ui->repeatButton->sizeHint());
    ui->volumeButton->setFixedSize(ui->volumeButton->sizeHint());
    ui->view->setFixedHeight(360);
    ui->songPlaylist->setFixedHeight(350);
    ui->view->setFixedWidth(340);

    // We might be starting NowPlayingWindow in portrait mode.
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() < screenGeometry.height()) {
        ui->horizontalLayout_3->setDirection(QBoxLayout::TopToBottom);
        ui->horizontalLayout_3->setDirection(QBoxLayout::TopToBottom);
        //if(!ui->volumeButton->isHidden())
            //ui->volWidget->hide();
        ui->space2->hide();
        ui->view->setFixedHeight(360);
        ui->view->setFixedWidth(470);
        ui->songPlaylist->setFixedHeight(290);
        //ui->songPlaylist->setFixedHeight(380);
        ui->volumeWidget->setContentsMargins(18,9,9,9);
        ui->buttonsLayout->setSpacing(27);
    }

    ui->lyrics->setFixedHeight(ui->songPlaylist->height());
    //new TextEditAutoResizer(ui->lyricsText);


    ui->toolBarWidget->setFixedHeight(80);
#ifdef MAFW
    mafw_playlist_manager = new MafwPlaylistManagerAdapter(this);
    if (mafwrenderer->isRendererReady()) {
        mafwrenderer->getCurrentMetadata();
        mafwrenderer->getStatus();
        mafwrenderer->getVolume();
    } else {
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getCurrentMetadata()));
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
        connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getVolume()));
    }

    data = new QNetworkAccessManager(this);
    connect(data, SIGNAL(finished(QNetworkReply*)), this, SLOT(onLyricsDownloaded(QNetworkReply*)));

    this->orientationChanged();

#endif
}

NowPlayingWindow::~NowPlayingWindow()
{
    delete ui;
}

void NowPlayingWindow::onLyricsDownloaded(QNetworkReply *reply)
{
    QString lyricsFile = reply->url().toString();
    //lyricsFile.remove("http://www.leoslyrics.com/").replace("/","-");
    lyricsFile.remove("http://lyrics.mirkforce.net/").replace("/", "-");

    //if ( ! lyricsFile.contains( ui->songTitleLabel->text().toLower().replace(" ","aeiou").remove(QRegExp("\\W")).replace("aeiou","-") ) )
    if ( ! lyricsFile.contains( cleanItem(ui->songTitleLabel->whatsThis()) ) )
        return;

    if (reply->error() != QNetworkReply::NoError) {
        //qDebug() << "error: " << reply->error();
        ui->lyricsText->setText(tr("Lyrics not found."));
        ui->lyricsText->setWhatsThis("");
    } else {
        QString lyrics = QString::fromUtf8(reply->readAll());
        //str.remove(QRegExp("(.*)oncontextmenu=\"return false;\">"));
        //str.remove(QRegExp("</p>(.*)"));
        //str.remove(QRegExp("<(.*)>"));
        ui->lyricsText->setText(lyrics);
        ui->lyricsText->setWhatsThis(lyricsFile);
        QString lyricsPath = "/home/user/.lyrics/";
        lyricsPath.append(lyricsFile);
        QFile file(lyricsPath);
        file.open( QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite );
        QTextStream out(&file);
        //out.setCodec("UTF-8");
        out << lyrics;
        file.close();
    }
}

void NowPlayingWindow::setAlbumImage(QString image)
{
    //qDebug() << image;

    qDeleteAll(albumArtScene->items());
    this->albumArtUri = image;
    if (image == albumImage)
    {
        // If there's no album art, search for it song's directory
        albumInFolder.remove("file://").replace("%20"," ");
        int x = albumInFolder.lastIndexOf("/");
        albumInFolder.remove(x, albumInFolder.length()-x);

        if ( QFileInfo(albumInFolder + "/cover.jpg").exists() )
            albumInFolder.append("/cover.jpg");
        else if ( QFileInfo(albumInFolder + "/front.jpg").exists() )
            albumInFolder.append("/front.jpg");
        else
            albumInFolder.append("/folder.jpg");

        if ( QFileInfo(albumInFolder).exists() )
        {
            image = MediaArt::setAlbumImage(ui->albumNameLabel->text(), albumInFolder);
            this->isDefaultArt = false;
        }
        else
            this->isDefaultArt = true;
    }
    else
        this->isDefaultArt = false;
    ui->view->setScene(albumArtScene);
    albumArtScene->setBackgroundBrush(QBrush(Qt::transparent));
    m = new mirror();
    albumArtScene->addItem(m);
    QPixmap albumArt(image);
    albumArt = albumArt.scaled(QSize(295, 295), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(albumArt);
    albumArtScene->addItem(item);
    m->setItem(item);
    /*QTransform t;
    t = t.rotate(-10, Qt::YAxis);
    ui->view->setTransform(t);*/
}

void NowPlayingWindow::setSongNumber(int currentSong, int numberOfSongs)
{
    QString songNumber;
    songNumber.append(QString::number(currentSong));
    songNumber.append("/");
    songNumber.append(QString::number(numberOfSongs));
    songNumber.append(" ");
    if (numberOfSongs == 1)
        songNumber.append(tr("song"));
    else
        songNumber.append(tr("songs"));
    ui->songNumberLabel->setText(songNumber);
}

void NowPlayingWindow::toggleVolumeSlider()
{
    if(ui->volSliderWidget->isHidden()) {
        ui->buttonsWidget->hide();
        ui->volSliderWidget->show();

    } else {
        ui->volSliderWidget->hide();
        ui->buttonsWidget->show();

        if(volumeTimer->isActive())
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
    this->setAlbumImage(albumImage);
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
    ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void NowPlayingWindow::metadataChanged(QString name, QVariant value)
{
#ifdef MAFW
    this->mafwrenderer->getCurrentMetadata();
#endif

    QFont f = ui->songTitleLabel->font();
    QFontMetrics fm(f);
    QString temp = fm.elidedText(value.toString(), Qt::ElideRight, 420);

    if(name == "title" /*MAFW_METADATA_KEY_TITLE*/)
        ui->songTitleLabel->setText(temp);
        ui->songTitleLabel->setWhatsThis(value.toString());
    if(name == "artist" /*MAFW_METADATA_KEY_ARTIST*/)
        ui->artistLabel->setText(temp);
        ui->artistLabel->setWhatsThis(value.toString());
    if(name == "album" /*MAFW_METADATA_KEY_ALBUM*/)
    {
        ui->albumNameLabel->setWhatsThis(value.toString());
        ui->albumNameLabel->setText(temp);
    }
    if(name == "uri")
        albumInFolder = value.toString();
    if(name == "renderer-art-uri")
        this->setAlbumImage(value.toString());
}

#ifdef MAFW
void NowPlayingWindow::stateChanged(int state)
{
    this->mafwState = state;

    if(state == Paused) {
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(resume()));
        mafwrenderer->getPosition();
        if(positionTimer->isActive())
            positionTimer->stop();
    }
    else if(state == Playing) {
        ui->playButton->setIcon(QIcon(pauseButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(pause()));
        mafwrenderer->getPosition();
        if(!positionTimer->isActive())
            positionTimer->start();
    }
    else if(state == Stopped) {
        ui->playButton->setIcon(QIcon(playButtonIcon));
        disconnect(ui->playButton, SIGNAL(clicked()), 0, 0);
        connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
        if(positionTimer->isActive())
            positionTimer->stop();
    }
    else if(state == Transitioning) {
        ui->songProgress->setEnabled(false);
        ui->songProgress->setValue(0);
        ui->songProgress->setRange(0, 99);
        ui->currentPositionLabel->setText("00:00");
    }
}
#endif

void NowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->actionFM_Transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionSave_playlist, SIGNAL(triggered()), this, SLOT(savePlaylist()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(volumeWatcher()));
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeSlider, SIGNAL(sliderPressed()), volumeTimer, SLOT(stop()));
    connect(ui->volumeSlider, SIGNAL(sliderReleased()), volumeTimer, SLOT(start()));
    connect(ui->view, SIGNAL(clicked()), this, SLOT(toggleList()));
    connect(ui->repeatButton, SIGNAL(clicked(bool)), this, SLOT(onRepeatButtonToggled(bool)));
    connect(ui->shuffleButton, SIGNAL(clicked(bool)), this, SLOT(onShuffleButtonToggled(bool)));
    connect(ui->nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));
    connect(ui->nextButton, SIGNAL(released()), this, SLOT(onNextButtonPressed()));
    connect(ui->prevButton, SIGNAL(pressed()), this, SLOT(onPrevButtonPressed()));
    connect(ui->prevButton, SIGNAL(released()), this, SLOT(onPrevButtonPressed()));
    connect(ui->songProgress, SIGNAL(sliderMoved(int)), this, SLOT(onPositionSliderMoved(int)));
    connect(ui->songPlaylist, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->actionEntertainment_view, SIGNAL(triggered()), this, SLOT(showEntertainmentView()));
    connect(ui->actionCar_view, SIGNAL(triggered()), this, SLOT(showCarView()));
#ifdef Q_WS_MAEMO_5
    connect(deviceEvents, SIGNAL(screenLocked(bool)), this, SLOT(onScreenLocked(bool)));
#endif
#ifdef MAFW
    connect(mafwrenderer, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString, QVariant)), this, SLOT(metadataChanged(QString, QVariant)));
    connect(mafwrenderer, SIGNAL(mediaChanged(int,char*)), this, SLOT(onMediaChanged(int, char*)));
    connect(mafwrenderer, SIGNAL(signalGetPosition(int,QString)), this, SLOT(onPositionChanged(int, QString)));
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(mafwrenderer, SIGNAL(signalGetCurrentMetadata(GHashTable*,QString,QString)),
            this, SLOT(onRendererMetadataRequested(GHashTable*,QString,QString)));
    connect(mafwrenderer, SIGNAL(mediaIsSeekable(bool)), ui->songProgress, SLOT(setEnabled(bool)));
    connect(mafwrenderer, SIGNAL(signalGetVolume(int)), ui->volumeSlider, SLOT(setValue(int)));
    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString,GHashTable*,QString)),
            this, SLOT(onSourceMetadataRequested(QString, GHashTable*, QString)));
    //connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint)), this, SLOT(onGetPlaylistItems(QString,GHashTable*,guint,gpointer)));
    connect(ui->volumeSlider, SIGNAL(sliderMoved(int)), mafwrenderer, SLOT(setVolume(int)));
    connect(ui->playButton, SIGNAL(clicked()), mafwrenderer, SLOT(play()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(onNextButtonClicked()));
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(onPreviousButtonClicked()));
    connect(positionTimer, SIGNAL(timeout()), mafwrenderer, SLOT(getPosition()));
    connect(ui->actionClear_now_playing, SIGNAL(triggered()), this, SLOT(clearPlaylist()));
    connect(ui->songPlaylist, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onPlaylistItemActivated(QListWidgetItem*)));
    connect(lastPlayingSong, SIGNAL(valueChanged()), this, SLOT(onGconfValueChanged()));

    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.extension",
                                          "property_changed",
                                          this, SLOT(onPropertyChanged(const QDBusMessage &)));
    QDBusConnection::sessionBus().connect("", "", "com.nokia.mafw.playlist", "property_changed",
                                          this, SLOT(updatePlaylistState()));
    QDBusConnection::sessionBus().connect("", "", "com.nokia.mafw.playlist", "playlist_updated", this, SLOT(updatePlaylist()));
    connect(playlist, SIGNAL(contentsChanged(guint, guint, guint)), this, SLOT(updatePlaylist(guint, guint, guint)));
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

void NowPlayingWindow::onSongSelected(int songNumber, int totalNumberOfSongs, QString song, QString album, QString artist, int duration)
{
    QFont f = ui->songTitleLabel->font();
    QFontMetrics fm(f);

    this->setAlbumImage(albumImage);
    ui->songNumberLabel->setText(QString::number(songNumber) + "/" + QString::number(totalNumberOfSongs) + tr(" songs"));
    ui->songTitleLabel->setWhatsThis(song);
    song = fm.elidedText(song, Qt::ElideRight, 420);
    ui->songTitleLabel->setText(song);
    ui->albumNameLabel->setWhatsThis(album);
    album = fm.elidedText(album, Qt::ElideRight, 420);
    ui->albumNameLabel->setText(album);
    artist = fm.elidedText(artist, Qt::ElideRight, 420);
    ui->artistLabel->setText(artist);
    this->songDuration = duration;
    ui->trackLengthLabel->setText(time_mmss(duration));
    ui->songPlaylist->setCurrentRow(songNumber-1);
    if(!ui->songPlaylist->isHidden()) {
        ui->songPlaylist->hide();
        ui->scrollArea->show();
    }
}

#ifdef MAFW
void NowPlayingWindow::onRendererMetadataRequested(GHashTable*, QString object_id, QString error)
{
    this->mafwTrackerSource->getMetadata(object_id.toUtf8(), MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_ALBUM,
                                                                              MAFW_METADATA_KEY_ARTIST,
                                                                              MAFW_METADATA_KEY_URI,
                                                                              MAFW_METADATA_KEY_ALBUM_ART_URI,
                                                                              MAFW_METADATA_KEY_RENDERER_ART_URI,
                                                                              MAFW_METADATA_KEY_DURATION,
                                                                              MAFW_METADATA_KEY_IS_SEEKABLE,
                                                                              MAFW_METADATA_KEY_LYRICS));
    if(!error.isNull() && !error.isEmpty())
        qDebug() << error;
}

void NowPlayingWindow::onSourceMetadataRequested(QString, GHashTable *metadata, QString error)
{
    QString title;
    QString artist;
    QString album;
    QString albumArt;
    bool isSeekable;
    int duration;

    if (metadata != NULL) {
        GValue *v;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_URI);
        albumInFolder = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : -1;
        this->songDuration = duration;

        QListWidgetItem* item = ui->songPlaylist->item(lastPlayingSong->value().toInt());
        if (item) {
            item->setData(UserRoleSongTitle, title);
            item->setData(UserRoleSongDuration, duration);
            item->setData(UserRoleSongAlbum, album);
            item->setData(UserRoleSongArtist, artist);
        }

        QFont f = ui->songTitleLabel->font();
        QFontMetrics fm(f);

        if ( newSong || (title!=ui->songTitleLabel->whatsThis()) )
        {

            v = mafw_metadata_first(metadata,
                                    MAFW_METADATA_KEY_IS_SEEKABLE);
            isSeekable = v ? g_value_get_boolean (v) : false;

            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
            if (v != NULL) {
                const gchar* file_uri = g_value_get_string(v);
                gchar* filename = NULL;
                if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                    this->setAlbumImage(QString::fromUtf8(filename));
                }
            } else {
                v = mafw_metadata_first(metadata,
                                        MAFW_METADATA_KEY_RENDERER_ART_URI);
                if (v != NULL) {
                    const gchar* file_uri = g_value_get_string(v);
                    gchar* filename = NULL;
                    if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                        this->setAlbumImage(QString::fromUtf8(filename));
                } else
                    this->setAlbumImage(albumImage);
            }

            if (enableLyrics) {
                v = mafw_metadata_first(metadata,
                                        MAFW_METADATA_KEY_LYRICS);
                QString lyrics = v ? QString::fromUtf8(g_value_get_string(v)) : "NOLYRICS";

                ui->lyricsText->setText(tr("Loading lyrics..."));
                ui->lyricsText->setWhatsThis("");
                QApplication::processEvents();

                QString lyricsPath = "/home/user/.lyrics/";
                QString lyricsFile = cleanItem(artist) + "-" + cleanItem(title) + ".txt";
                QDir d;
                d.mkdir(lyricsPath);
                lyricsPath.append(lyricsFile);
                if ( QFileInfo(lyricsPath).exists() )
                {
                    QString lines;
                    QFile data(lyricsPath);
                    if ( data.open(QFile::ReadOnly) )
                    {
                        QTextStream out(&data);
                        while ( !out.atEnd() )
                            lines += out.readLine()+"\n";
                    }
                    data.close();
                    ui->lyricsText->setText(lines);
                    ui->lyricsText->setWhatsThis(lyricsFile);
                    QApplication::processEvents();
                    //qDebug() << "loading from file..." << lyricsPath;
                }
                else
                {
                    QNetworkConfigurationManager mgr;
                    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
                    if ( activeConfigs.count() > 0 )
                    {
                        ui->lyricsText->setText(tr("Fetching lyrics..."));
                        ui->lyricsText->setWhatsThis("");
                        QApplication::processEvents();

                        data->get( QNetworkRequest( QUrl( "http://lyrics.mirkforce.net/" + lyricsFile.replace("-", "/") ) ) );

                        /*QString salida = "http://www.leoslyrics.com/";
                        salida += artist.toLower().replace(" ","aeiou").remove(QRegExp("\\W")).replace("aeiou","-") + "/";
                        salida += title.toLower().replace(" ","aeiou").remove(QRegExp("\\W")).replace("aeiou","-") + "-lyrics/" ;
                        data->get(QNetworkRequest(QUrl(salida)));*/
                        //qDebug() << salida;

                    } else
                    {
                        QString line1 = tr("There is no active Internet connection.");
                        QString line2 = tr("Please connect to use this function.");
                        ui->lyricsText->setText(line1+"\n"+line2);
                    }
                }
            }

            ui->songTitleLabel->setWhatsThis(title);
            title = fm.elidedText(title, Qt::ElideRight, 420);
            ui->songTitleLabel->setText(title);

            ui->artistLabel->setWhatsThis(artist);
            artist = fm.elidedText(artist, Qt::ElideRight, 420);
            ui->artistLabel->setText(artist);

            ui->albumNameLabel->setWhatsThis(album);
            album = fm.elidedText(album, Qt::ElideRight, 420);
            ui->albumNameLabel->setText(album);

            ui->trackLengthLabel->setText(time_mmss(duration));
            ui->songProgress->setRange(0, duration);
            if (isSeekable)
                ui->songProgress->setEnabled(true);

            newSong = false;

        }

        this->updateEntertainmentViewMetadata();
        this->updateCarViewMetadata();

        if(!error.isNull() && !error.isEmpty())
        qDebug() << error;

    }

}

#endif

void NowPlayingWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        // Landscape mode
#ifdef DEBUG
        qDebug() << "NowPlayingWindow: Orientation changed: Landscape.";
#endif
        ui->horizontalLayout_3->setDirection(QBoxLayout::LeftToRight);
        if(ui->volWidget->isHidden())
            ui->volWidget->show();
        ui->view->setFixedHeight(360);
        ui->songPlaylist->setFixedHeight(350);
        ui->view->setFixedWidth(340);
        ui->volumeWidget->setContentsMargins(0,9,9,9);
        ui->space2->show();
        ui->buttonsLayout->setSpacing(60);

        ui->songNumberLabel->setAlignment(Qt::AlignLeft);
        ui->songTitleLabel->setAlignment(Qt::AlignLeft);
        ui->artistLabel->setAlignment(Qt::AlignLeft);
        ui->albumNameLabel->setAlignment(Qt::AlignLeft);
        ui->lyricsText->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    } else {
        // Portrait mode
#ifdef DEBUG
        qDebug() << "NowPlayingWindow: Orientation changed: Portrait.";
#endif
        ui->horizontalLayout_3->setDirection(QBoxLayout::TopToBottom);
        if(!ui->volumeButton->isHidden())
            ui->volWidget->hide();

        ui->space2->hide();
        ui->view->setFixedHeight(360);
        ui->view->setFixedWidth(470);
        ui->songPlaylist->setFixedHeight(290);
        ui->volumeWidget->setContentsMargins(18,9,9,9);
        ui->buttonsLayout->setSpacing(27);

        ui->songNumberLabel->setAlignment(Qt::AlignHCenter);
        ui->songTitleLabel->setAlignment(Qt::AlignHCenter);
        ui->artistLabel->setAlignment(Qt::AlignHCenter);
        ui->albumNameLabel->setAlignment(Qt::AlignHCenter);
        ui->lyricsText->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }
    ui->lyrics->setFixedHeight(ui->songPlaylist->height());
}

void NowPlayingWindow::toggleList()
{
    if(ui->songPlaylist->isHidden() && ui->lyrics->isHidden()) {
        ui->widget->hide();
        ui->lyrics->hide();
        ui->songPlaylist->show();
        ui->spacer1->hide();
#ifdef MAFW
        positionTimer->stop();
#endif
    } else if(enableLyrics && ui->lyrics->isHidden() && ui->widget->isHidden()) {
        ui->widget->hide();
        ui->songPlaylist->hide();
        ui->lyrics->show();
        ui->spacer1->hide();
#ifdef MAFW
        positionTimer->stop();
#endif
    } else {
        ui->lyrics->hide();
        ui->songPlaylist->hide();
        ui->widget->show();
        ui->spacer1->show();
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
    if(!ui->volSliderWidget->isHidden())
        volumeTimer->start();
}

void NowPlayingWindow::onShuffleButtonToggled(bool checked)
{
    if(checked) {
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
    if(checked) {
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
    if(ui->nextButton->isDown()) {
        ui->nextButton->setIcon(QIcon(nextButtonPressedIcon));
    } else {
        ui->nextButton->setIcon(QIcon(nextButtonIcon));
    }
}

void NowPlayingWindow::onPrevButtonPressed()
{
    if(ui->prevButton->isDown())
        ui->prevButton->setIcon(QIcon(prevButtonPressedIcon));
    else
        ui->prevButton->setIcon(QIcon(prevButtonIcon));
}

void NowPlayingWindow::onPositionSliderMoved(int position)
{
#ifdef MAFW
    mafwrenderer->setPosition(SeekAbsolute, position);
    ui->currentPositionLabel->setText(time_mmss(position));
#endif
}

#ifdef MAFW
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
    if (!ui->songProgress->isSliderDown())
        ui->currentPositionLabel->setText(time_mmss(position));

    if (this->songDuration != 0 && this->songDuration != -1 && entertainmentView == 0 && carView == 0) {
#ifdef DEBUG
        qDebug() << "Current position: " << position;
        qDebug() << "Song Length: " << this->songDuration;
#endif
        if (!ui->songProgress->isSliderDown() && ui->songProgress->isVisible())
            ui->songProgress->setValue(position);
    }
}

void NowPlayingWindow::onGetStatus(MafwPlaylist* MafwPlaylist, uint index, MafwPlayState state, const char *, QString)
{
    if (!this->playlistRequested) {
        this->updatePlaylist();
        this->updatePlaylistState();
        this->playlistRequested = true;
    }
    int indexAsInt = index;
    lastPlayingSong->set(indexAsInt);
    this->mafwPlaylist = MafwPlaylist;
    this->setSongNumber(index+1, playlist->getSize());
    this->stateChanged(state);
}

void NowPlayingWindow::setPosition(int newPosition)
{
    mafwrenderer->setPosition(SeekAbsolute, newPosition);
    mafwrenderer->getPosition();
}

#ifdef MAFW
void NowPlayingWindow::showEvent(QShowEvent *)
{
    mafwrenderer->getCurrentMetadata();
    mafwrenderer->getStatus();
    this->updatePlaylistState();
    if (positionTimer->isActive())
        ui->songProgress->setEnabled(true);
}

void NowPlayingWindow::onGconfValueChanged()
{
    this->setSongNumber(lastPlayingSong->value().toInt()+1, ui->songPlaylist->count());
    this->selectItemByRow(lastPlayingSong->value().toInt());
}

void NowPlayingWindow::onMediaChanged(int index, char*)
{
    newSong = true;
    lastPlayingSong->set(index);
    this->isDefaultArt = true;
}

#endif

#endif

void NowPlayingWindow::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Backspace)
        this->close();
#ifdef MAFW
    else if(e->key() == Qt::Key_Space) {
        if (this->mafwState == Playing)
            mafwrenderer->pause();
        else if (this->mafwState == Paused)
            mafwrenderer->resume();
        else if (this->mafwState == Stopped)
            mafwrenderer->play();
    }
    else if (e->key() == Qt::Key_Right)
        mafwrenderer->next();
    else if (e->key() == Qt::Key_Left)
        mafwrenderer->previous();
#endif
    /*else if(e->key() == Qt::Key_Shift) {
        if(ui->menuNow_playing_menu->isHidden())
            ui->menuNow_playing_menu->show();
        else
            ui->menuNow_playing_menu->hide();
    }*/
}

#ifdef MAFW
void NowPlayingWindow::onGetPlaylistItems(QString object_id, GHashTable *metadata, guint index, gpointer op)
{
    qDebug() << "NowPlayingWindow::onGetPlaylistItems | index: " << index;
    if (--numberOfSongsToAdd == 0) {
        int e = t.elapsed();
        qDebug() << QString("%1 ms, %2/s").arg(e).arg((double)n/e*1000);

        qDebug() << "disconnecting NowPlayingWindow from onGetItems";
        disconnect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
                   this, SLOT(onGetPlaylistItems(QString,GHashTable*,guint,gpointer)));
        browseId = NULL;
    } else if (numberOfSongsToAdd % BATCH_SIZE == 0)
        browseId = playlist->getItems(numberOfSongsToAdd-BATCH_SIZE, numberOfSongsToAdd-1);

    QString title;
    QString artist;
    QString album;
    int duration;
    if (metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");


        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        QListWidgetItem *item =  ui->songPlaylist->item(index);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, object_id);
        item->setData(UserRoleSongIndex, index);

        /*unsigned theIndex = 0;
        int position;
        for (position = 0; position < ui->songPlaylist->count(); position++)
        {
            theIndex = ui->songPlaylist->item(position)->data(UserRoleSongIndex).toInt();
            if (theIndex > index)
                break;
        }*/

        //ui->songPlaylist->insertItem(position, item);
        this->setSongNumber(lastPlayingSong->value().toInt()+1, ui->songPlaylist->count()-numberOfSongsToAdd);
    } else qDebug() << "warning: null metadata";
}

void NowPlayingWindow::onPlaylistItemActivated(QListWidgetItem *item)
{
#ifdef DEBUG
    qDebug() << "Selected item number: " << ui->songPlaylist->currentRow();
    //qDebug() << "Item number in MAFW playlist: " << item->text();
#endif
    this->setSongNumber(ui->songPlaylist->currentRow()+1, ui->songPlaylist->count());
    lastPlayingSong->set(ui->songPlaylist->currentRow());

    QFont f = ui->songTitleLabel->font();
    QFontMetrics fm(f);
    ui->songTitleLabel->setWhatsThis(item->data(UserRoleSongTitle).toString());
    QString temp = fm.elidedText(item->data(UserRoleSongTitle).toString(), Qt::ElideRight, 420);
    ui->songTitleLabel->setText(temp);
    ui->artistLabel->setWhatsThis(item->data(UserRoleSongArtist).toString());
    temp = fm.elidedText(item->data(UserRoleSongArtist).toString(), Qt::ElideRight, 420);
    ui->artistLabel->setText(temp);
    ui->albumNameLabel->setWhatsThis(item->data(UserRoleSongAlbum).toString());
    temp = fm.elidedText(item->data(UserRoleSongAlbum).toString(), Qt::ElideRight, 420);
    ui->albumNameLabel->setText(temp);
    ui->currentPositionLabel->setText("00:00");
    this->songDuration = item->data(UserRoleSongDuration).toInt();
    ui->trackLengthLabel->setText(time_mmss(songDuration));
    mafwrenderer->gotoIndex(ui->songPlaylist->row(item));
    if (this->mafwState == Stopped)
        mafwrenderer->play();
}

void NowPlayingWindow::updatePlaylistState()
{    
    if(playlist->isShuffled()) {
        ui->shuffleButton->setIcon(QIcon(shuffleButtonPressed));
        ui->shuffleButton->setChecked(true);
    } else {
        ui->shuffleButton->setIcon(QIcon(shuffleButtonIcon));
        ui->shuffleButton->setChecked(false);
    }

    if(playlist->isRepeat()) {
        ui->repeatButton->setIcon(QIcon(repeatButtonPressedIcon));
        ui->repeatButton->setChecked(true);
    } else {
        ui->repeatButton->setIcon(QIcon(repeatButtonIcon));
        ui->repeatButton->setChecked(false);
    }
}

void NowPlayingWindow::clearPlaylist()
{
    QMessageBox confirmClear(QMessageBox::NoIcon,
                             " ",
                             tr("Clear all songs from now playing?"),
                             QMessageBox::Yes | QMessageBox::No,
                             this);
    confirmClear.exec();
    if(confirmClear.result() == QMessageBox::Yes) {
        playlist->clear();
        lastPlayingSong->set(1);
        mafwrenderer->stop();
        this->close();
    }
}

void NowPlayingWindow::onPlaylistChanged()
{
    qDebug() << "NowPlayingWindow::onPlaylistChanged";
    for (int i = 0; i < ui->songPlaylist->count(); i++) {
        QListWidgetItem *item = ui->songPlaylist->item(i);
        ui->songPlaylist->removeItemWidget(item);
        delete item;
    }
    this->updatePlaylist();
}
#endif

void NowPlayingWindow::onContextMenuRequested(const QPoint &point)
{
    contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Save playlist"), this, SLOT(savePlaylist()));
    //contextMenu->addAction(tr("Edit tags"), this, SLOT(editTags()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    contextMenu->addAction(tr("Delete from now playing"), this, SLOT(onDeleteFromNowPlaying()));
    contextMenu->addAction(tr("Clear now playing"), this, SLOT(clearPlaylist()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void NowPlayingWindow::setRingingTone()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songPlaylist->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void NowPlayingWindow::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songPlaylist->currentItem()->data(UserRoleObjectID).toString())
        return;

#ifdef Q_WS_MAEMO_5
    QDBusInterface setRingtone("com.nokia.profiled",
                               "/com/nokia/profiled",
                               "com.nokia.profiled",
                               QDBusConnection::sessionBus(), this);
    setRingtone.call("set_value", "general", "ringing.alert.tone", uri);
    QMaemo5InformationBox::information(this, "Selected song set as ringing tone");
#endif
}
#endif

void NowPlayingWindow::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songPlaylist->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void NowPlayingWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songPlaylist->currentItem()->data(UserRoleObjectID).toString())
        return;

    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
    QStringList list;
    QString clip;
    clip = uri;
#ifdef DEBUG
    qDebug() << "Sending file:" << clip;
#endif
    list.append(clip);
#ifdef Q_WS_MAEMO_5
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
#endif
}
#endif

void NowPlayingWindow::showEntertainmentView()
{
    entertainmentView = new EntertainmentView(this, mafwFactory);
    entertainmentView->setAttribute(Qt::WA_DeleteOnClose);
    for (int i = 0; i < ui->songPlaylist->count(); i++) {
        entertainmentView->addItemToPlaylist(ui->songPlaylist->item(i), i);
    }
    connect(entertainmentView, SIGNAL(destroyed()), this, SLOT(nullEntertainmentView()));
    this->updateEntertainmentViewMetadata();
    entertainmentView->showFullScreen();
}

void NowPlayingWindow::showCarView()
{
    carView = new CarView(this, mafwFactory);
    carView->setAttribute(Qt::WA_DeleteOnClose);
    for (int i = 0; i < ui->songPlaylist->count(); i++) {
        carView->addItemToPlaylist(ui->songPlaylist->item(i), i);
    }
    connect(carView, SIGNAL(destroyed()), this, SLOT(nullCarView()));
    this->updateCarViewMetadata();
    carView->showFullScreen();
}

void NowPlayingWindow::updateEntertainmentViewMetadata()
{
    if (entertainmentView) {
        entertainmentView->setMetadata(ui->songTitleLabel->text(),
                                       ui->albumNameLabel->text(),
                                       ui->artistLabel->text(),
                                       this->albumArtUri,
                                       this->songDuration);
        entertainmentView->setCurrentRow(ui->songPlaylist->row(ui->songPlaylist->currentItem()));
    }
}

void NowPlayingWindow::updateCarViewMetadata()
{
    if (carView) {
        carView->setMetadata(ui->songTitleLabel->text(),
                                       ui->albumNameLabel->text(),
                                       ui->artistLabel->text(),
                                       this->albumArtUri,
                                       this->songDuration);
        carView->setCurrentRow(ui->songPlaylist->row(ui->songPlaylist->currentItem()));
    }
}

void NowPlayingWindow::nullEntertainmentView()
{
    entertainmentView = 0;
#ifdef MAFW
    mafwrenderer->getPosition();
#endif
}

void NowPlayingWindow::nullCarView()
{
    carView = 0;
#ifdef MAFW
    mafwrenderer->getPosition();
#endif
}

void NowPlayingWindow::savePlaylist()
{
    savePlaylistDialog = new QDialog(this);
    savePlaylistDialog->setWindowTitle(tr("Save playlist"));
    savePlaylistDialog->setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout *layout = new QGridLayout(savePlaylistDialog);

    QLabel *nameLabel = new QLabel(savePlaylistDialog);
    nameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setText(tr("Name"));

    playlistNameLineEdit = new QLineEdit(savePlaylistDialog);
    playlistNameLineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, Qt::Horizontal, this);
    buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSavePlaylistAccepted()));

    layout->addWidget(nameLabel, 0, 0);
    layout->addWidget(playlistNameLineEdit, 0, 1);
    layout->addWidget(buttonBox, 0, 2);

    savePlaylistDialog->show();
}

void NowPlayingWindow::onSavePlaylistAccepted()
{
#ifdef MAFW
    bool playlistExists = false;
    GArray* playlists = mafw_playlist_manager->listPlaylists();
    QString playlistName;

    for (uint i = 0; i < playlists->len; i++) {
        MafwPlaylistManagerItem* item = &g_array_index(playlists, MafwPlaylistManagerItem, i);

        playlistName = QString::fromUtf8(item->name);
        if (playlistName == playlistNameLineEdit->text())
            playlistExists = true;
    }

    if (playlistExists) {
        QMessageBox overwrite(QMessageBox::NoIcon,
                              " ",
                              tr("Playlist with the same name exists, overwrite?"),
                              QMessageBox::Yes | QMessageBox::No,
                              savePlaylistDialog);
        overwrite.exec();
        if (overwrite.result() == QMessageBox::Yes) {
            mafw_playlist_manager->deletePlaylist(playlistNameLineEdit->text());
            playlist->duplicatePlaylist(playlistNameLineEdit->text());
            savePlaylistDialog->close();
        }
        else if (overwrite.result() == QMessageBox::No) {
            overwrite.close();
#ifdef Q_WS_MAEMO_5
            QMaemo5InformationBox::information(this, tr("Playlist not saved"));
#endif
        }
    }       else {
        playlist->duplicatePlaylist(playlistNameLineEdit->text());
        savePlaylistDialog->close();
    }
#else
    savePlaylistDialog->close();
#endif
}

void NowPlayingWindow::onDeleteFromNowPlaying()
{
#ifdef MAFW
    playlist->removeItem(ui->songPlaylist->currentRow());
#endif
}

/*void NowPlayingWindow::selectItemByText(int numberInPlaylist)
{
    for (int i = 0; i < ui->songPlaylist->count(); i++) {
        if (ui->songPlaylist->item(i)->text().toInt() == numberInPlaylist) {
            ui->songPlaylist->clearSelection();
            ui->songPlaylist->item(i)->setSelected(true);
            ui->songPlaylist->setCurrentItem(ui->songPlaylist->item(i));
            ui->songPlaylist->scrollToItem(ui->songPlaylist->item(i), QAbstractItemView::PositionAtCenter);
        }
    }
}*/

void NowPlayingWindow::selectItemByRow(int row)
{
    if (ui->songPlaylist->item(row)){
        ui->songPlaylist->clearSelection();
        ui->songPlaylist->item(row)->setSelected(true);
        ui->songPlaylist->setCurrentRow(row);
        ui->songPlaylist->scrollToItem(ui->songPlaylist->item(row), QAbstractItemView::PositionAtCenter);
    }
}

void NowPlayingWindow::updatePlaylist(guint from, guint nremove, guint nreplace)
{
    qDebug() << "NowPlayingWindow::updatePlaylist |" << from << nremove << nreplace;

    if (playlist->playlistName() != "FmpAudioPlaylist") {
        qDebug() << "playlist type rejected, update aborted";
        return;
    }

    // measure playlist fill rate
    n = numberOfSongsToAdd;
    t.setHMS(0,0,0,0);
    t.start();

    if (nreplace < BATCH_SIZE && from + nremove + nreplace != 0 && numberOfSongsToAdd == 0) {
        if (nremove > 0)
            for (int i = 0; i < nremove; i++)
                delete ui->songPlaylist->takeItem(from);
        else {
            for (int i = 0; i < nreplace; i++) {
                QListWidgetItem *item = new QListWidgetItem();
                //item->setText(QString::number(i));
                item->setData(UserRoleValueText, " ");
                item->setData(UserRoleSongDuration, Duration::Blank);
                ui->songPlaylist->insertItem(from, item);
            }

            qDebug() << "connecting SinglePlaylistView to onGetItems";
            connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
                    this, SLOT(onGetPlaylistItems(QString,GHashTable*,guint,gpointer)), Qt::UniqueConnection);

            numberOfSongsToAdd = nreplace;
            playlist->getItems(from, from+nreplace);
        }

    } else {

        if (browseId) {
            qDebug() << "cancelling previous get_items query";
            mafw_playlist_cancel_get_items_md(browseId);
        }

        ui->songPlaylist->clear();

        if (numberOfSongsToAdd = playlist->getSize()) {
            for (int i = 0; i < numberOfSongsToAdd; i++) {
                QListWidgetItem *item = new QListWidgetItem(ui->songPlaylist);
                //item->setText(QString::number(i));
                item->setData(UserRoleValueText, " ");
                item->setData(UserRoleSongDuration, Duration::Blank);
                ui->songPlaylist->addItem(item);
            }

            qDebug() << "connecting SinglePlaylistView to onGetItems";
            connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
                    this, SLOT(onGetPlaylistItems(QString,GHashTable*,guint,gpointer)), Qt::UniqueConnection);

            // For some reason MAFW doesn't send metadata when handling too many
            // requests. The workaround here is to query a safe number of items and
            // wait with another batch until all of them are served.

            // Smaller batches make playlist filling more fluid, but it takes more
            // time to complete. Large batches might be not safe.

            browseId = playlist->getItems(numberOfSongsToAdd-1 - (numberOfSongsToAdd-1)%BATCH_SIZE, -1);

            // fast and easy, but not safe and results come late
            /*browseId = playlist->getAllItems();*/

            // fast and results start to come early, but not safe
            /*for (int i = 0; i < numberOfSongsToAdd; i+=10)
                playlist->getItems(i, i+10);*/
        }
    }

    mafwrenderer->getStatus();
}

void NowPlayingWindow::on_view_customContextMenuRequested(QPoint pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Select album art"), this, SLOT(selectAlbumArt()));
    contextMenu->addAction(tr("Reset album art"), this, SLOT(resetAlbumArt()));
    contextMenu->exec(mapToGlobal(pos));
}

void NowPlayingWindow::selectAlbumArt()
{
    Home* hw = new Home(this, tr("Select album art"), "/home/user/MyDocs", ui->albumNameLabel->whatsThis());
    hw->exec();
    if ( hw->result() == QDialog::Accepted )
        setAlbumImage(hw->newAlbumArt);
    delete hw;
}

void NowPlayingWindow::resetAlbumArt()
{
    QMessageBox confirm(QMessageBox::NoIcon,
                             " ",
                             tr("Reset album art?"),
                             QMessageBox::Yes | QMessageBox::No,
                             this);
    confirm.exec();
    if(confirm.result() == QMessageBox::Yes)
        setAlbumImage(MediaArt::setAlbumImage(ui->albumNameLabel->text(), ""));
}

void NowPlayingWindow::on_lyricsText_customContextMenuRequested(QPoint pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Edit lyrics"), this, SLOT(editLyrics()));
    contextMenu->addAction(tr("Reload lyrics"), this, SLOT(reloadLyrics()));
    contextMenu->exec(mapToGlobal(pos));

}

void NowPlayingWindow::editLyrics()
{
    if ( ui->lyricsText->whatsThis() == "" )
    {
        QString str = cleanItem(ui->artistLabel->whatsThis()) + "-" +
                      cleanItem(ui->songTitleLabel->whatsThis()) + ".txt";
        ui->lyricsText->setWhatsThis(str);
    }

    if ( ui->lyricsText->whatsThis() != "" )
    {
        EditLyrics* el = new EditLyrics(this, ui->lyricsText->whatsThis(),
                                        ui->artistLabel->whatsThis(), ui->songTitleLabel->whatsThis() );
        el->show();
    }

}

void NowPlayingWindow::reloadLyricsFromFile()
{
    QString lines = "";
    QString file = "/home/user/.lyrics/" + ui->lyricsText->whatsThis();
    QFile data(file);
    if (data.open(QFile::ReadOnly))
    {
        QTextStream out(&data);
        while ( !out.atEnd() )
            lines += out.readLine()+"\n";
    }
    data.close();
    QApplication::processEvents();
    ui->lyricsText->setText(lines);
}

void NowPlayingWindow::reloadLyrics()
{
    //qDebug() << ui->lyricsText->whatsThis();
    if ( ui->lyricsText->whatsThis() != "" )
    {
        data->get( QNetworkRequest( QUrl( "http://lyrics.mirkforce.net/" + ui->lyricsText->whatsThis().replace("-", "/") ) ) );
        ui->lyricsText->setText(tr("Fetching lyrics..."));
        ui->lyricsText->setWhatsThis("");
    }
}

QString NowPlayingWindow::cleanItem(QString data)
{
    data = data.toLower().replace("&","and");
    int i = data.indexOf("(");
    if ( i > 0 )
    {
        int j = data.indexOf(")");
        if ( j > 0 ) data.remove(i, j);
    }
    data = data.remove(QRegExp("[\\W_]"));
    //qDebug() << data;
    return data;
}

void NowPlayingWindow::editTags()
{
    TEid = ui->songPlaylist->currentItem()->data(UserRoleObjectID).toString();
    TagWindow* tw = new TagWindow(this, ui->songPlaylist->currentItem()->data(UserRoleObjectID).toString(),
                                  ui->songPlaylist->currentItem()->data(UserRoleSongArtist).toString(),
                                  ui->songPlaylist->currentItem()->data(UserRoleSongAlbum).toString(),
                                  ui->songPlaylist->currentItem()->data(UserRoleSongTitle).toString());
    int result = tw->exec();

    if ( result == QDialog::Accepted )
    {
        TEartist = tw->artist;
        TEalbum = tw->album;
        TEtitle = tw->title;

        qDebug() << TEartist << TEalbum << TEtitle;
        GHashTable* hash = g_hash_table_new(g_str_hash, g_str_equal);
        const gchar *val1 = MAFW_METADATA_KEY_TITLE;
        const gchar *val2 = TEtitle.toUtf8();
        g_hash_table_insert(hash, &val1, &val2);
        this->mafwTrackerSource->setMetadata(TEid.toUtf8(),hash);
        g_hash_table_destroy(hash);

    }
}

void NowPlayingWindow::closeEvent(QCloseEvent *e)
{
    this->hide();
    this->setParent(0);
    emit hidden();
    e->ignore();
}
