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

#include "texteditautoresizer.h"
#include "nowplayingwindow.h"
#include "home.h"
#include "editlyrics.h"
#include "tagwindow.h"
#include "hildon-thumbnail/hildon-albumart-factory.h"
#include <QNetworkConfigurationManager>

int newsong;
QString currentPlaylistUrl;

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

    newsong = 1;
    albumArtScene = new QGraphicsScene(ui->view);
    entertainmentView = 0;

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
    connect(data, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloaded(QNetworkReply*)));

    this->orientationChanged();

#endif
}

NowPlayingWindow::~NowPlayingWindow()
{
    delete ui;
}

void NowPlayingWindow::downloaded(QNetworkReply *reply)
{
    QString f1 = reply->url().toString();
    //f1.remove("http://www.leoslyrics.com/").replace("/","-");
    f1.remove("http://lyrics.mirkforce.net/");

    //if ( ! f1.contains( ui->songTitleLabel->text().toLower().replace(" ","aeiou").remove(QRegExp("\\W")).replace("aeiou","-") ) )
    if ( ! f1.contains( cleanItem(ui->songTitleLabel->whatsThis()) ) )
        return;

    if (reply->error() != QNetworkReply::NoError) {
        //qDebug() << "error: " << reply->error();
        ui->lyricsText->setText(tr("No lyrics founded"));
        ui->lyricsText->setWhatsThis("");
    } else {
        QString str = QString::fromUtf8(reply->readAll());
        //str.remove(QRegExp("(.*)oncontextmenu=\"return false;\">"));
        //str.remove(QRegExp("</p>(.*)"));
        //str.remove(QRegExp("<(.*)>"));
        //qDebug() << str.toLatin1();
        ui->lyricsText->setText(str);
        ui->lyricsText->setWhatsThis(f1);
        QString f = "/home/user/.lyrics/";
        f1.replace("/","-");
        f.append(f1);
        if ( QFileInfo(f).exists() )
            QFile::remove(f);
        QFile file(f);
        file.open( QIODevice::Truncate | QIODevice::Text | QIODevice::ReadWrite);
        QTextStream out(&file);
        //out.setCodec("UTF-8");
        out << str;
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
        //If there's no albumart search for folder.jpg file and apply it
        albuminfolder.remove("file://").replace("%20"," ");
        int x = albuminfolder.lastIndexOf("/");
        albuminfolder.remove(x,albuminfolder.length()-x);
        albuminfolder.append("/folder.jpg");
        if ( QFileInfo(albuminfolder).exists() )
        {
            QString newfile = QString::fromUtf8(hildon_albumart_get_path(NULL,ui->albumNameLabel->whatsThis().toUtf8(),"album"));
            if ( QFileInfo(newfile).exists() )
                QFile::remove(newfile);
            QFile::copy(albuminfolder,newfile);
            image = albuminfolder;
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
    albumArt = albumArt.scaled(QSize(295, 295));
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
        albuminfolder = value.toString();
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
    connect(ui->actionEntertainment_view, SIGNAL(triggered()), this, SLOT(showEntertainmentview()));
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
    connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint)), this, SLOT(onGetPlaylistItems(QString,GHashTable*,guint)));
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
    QDBusConnection::sessionBus().connect("", "", "com.nokia.mafw.playlist", "contents_changed", this, SLOT(updatePlaylist()));
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
    QTime t(0, 0);
    t = t.addSecs(duration);
    ui->trackLengthLabel->setText(t.toString("mm:ss"));
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
    QString lyrics;
    bool isSeekable;
    int duration = -1;
    QTime t(0, 0);
    if(metadata != NULL) {
        GValue *v;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_URI);
        albuminfolder = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : -1;
        t = t.addSecs(duration);
        this->songDuration = duration;

        QFont f = ui->songTitleLabel->font();
        QFontMetrics fm(f);

        if ( (newsong==1) || (title!=ui->songTitleLabel->whatsThis()) )
        {

            v = mafw_metadata_first(metadata,
                                    MAFW_METADATA_KEY_IS_SEEKABLE);
            isSeekable = v ? g_value_get_boolean (v) : false;

            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
            if(v != NULL) {
                const gchar* file_uri = g_value_get_string(v);
                gchar* filename = NULL;
                if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                    this->setAlbumImage(QString::fromUtf8(filename));
                }
            } else {
                v = mafw_metadata_first(metadata,
                                        MAFW_METADATA_KEY_RENDERER_ART_URI);
                if(v != NULL) {
                    const gchar* file_uri = g_value_get_string(v);
                    gchar* filename = NULL;
                    if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                        this->setAlbumImage(QString::fromUtf8(filename));
                } else
                    this->setAlbumImage(albumImage);
            }

            v = mafw_metadata_first(metadata,
                                    MAFW_METADATA_KEY_LYRICS);
            lyrics = v ? QString::fromUtf8(g_value_get_string(v)) : "NOLYRICS";

            ui->lyricsText->setText(tr("Loading lyrics..."));
            ui->lyricsText->setWhatsThis("");
            QApplication::processEvents();
            QDir d;
            d.mkdir("/home/user/.lyrics");
            QString fi = "/home/user/.lyrics/";
            QString fj = cleanItem(artist) + "/" + cleanItem(title) + ".txt";
            fi.append(fj.replace("/","-"));
            if ( QFileInfo(fi).exists() )
            {
                QString lines;
                QFile data(fi);
                if (data.open(QFile::ReadOnly | QFile::Truncate))
                {
                    QTextStream out(&data);
                    while ( !out.atEnd() )
                        lines += out.readLine()+"\n";
                }
                data.close();
                QApplication::processEvents();
                ui->lyricsText->setText(lines);
                ui->lyricsText->setWhatsThis(fj);
                //qDebug() << "loading from file..." << fi;
            }
            else
            {
                QNetworkConfigurationManager mgr;
                QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);
                if ( activeConfigs.count() > 0 )
                {
                    ui->lyricsText->setText(tr("Fetching lyrics..."));
                    ui->lyricsText->setWhatsThis("");
                    data->get(QNetworkRequest(QUrl("http://lyrics.mirkforce.net/" +
                                                   cleanItem(artist) + "/" + cleanItem(title) + ".txt")));

                    /*QString salida = "http://www.leoslyrics.com/";
                    salida += artist.toLower().replace(" ","aeiou").remove(QRegExp("\\W")).replace("aeiou","-") + "/";
                    salida += title.toLower().replace(" ","aeiou").remove(QRegExp("\\W")).replace("aeiou","-") + "-lyrics/" ;
                    data->get(QNetworkRequest(QUrl(salida)));*/
                    //qDebug() << salida;

                } else
                {
                    QString line1 = tr("There is not an active Internet connection");
                    QString line2 = tr("Please connect to use this function.");
                    ui->lyricsText->setText(line1+"\n"+line2);
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

            ui->trackLengthLabel->setText(t.toString("mm:ss"));
            ui->songProgress->setRange(0, duration);
            if (isSeekable)
                ui->songProgress->setEnabled(true);

            newsong=0;

        }

        this->updateEntertainmentViewMetadata();

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
    } else if(ui->lyrics->isHidden() && ui->widget->isHidden()) {
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
    QTime t(0, 0);
    t = t.addSecs(position);
    ui->currentPositionLabel->setText(t.toString("mm:ss"));
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
    QTime t(0, 0);
    t = t.addSecs(position);
    if (!ui->songProgress->isSliderDown())
        ui->currentPositionLabel->setText(t.toString("mm:ss"));

    if (this->songDuration != 0 && this->songDuration != -1 && entertainmentView == 0) {
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
    this->selectItemByText(lastPlayingSong->value().toInt());
}

void NowPlayingWindow::onMediaChanged(int index, char*)
{
    newsong = 1;
    lastPlayingSong->set(index);
    this->selectItemByText(index);
    ui->songPlaylist->scrollToItem(ui->songPlaylist->item(index));
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
void NowPlayingWindow::onGetPlaylistItems(QString object_id, GHashTable *metadata, guint index)
{
    QString title;
    QString artist;
    QString album;
    int duration = -1;
    if(metadata != NULL) {
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
        duration = v ? g_value_get_int (v) : -1;

        QListWidgetItem *item = ui->songPlaylist->item(index);
        item->setText(QString::number(index));
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, object_id);
        item->setData(UserRoleSongIndex, index);

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        //qDebug() << g_value_get_string(mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI));
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleSongURI, QString::fromUtf8(filename));
            }
        }
        // SHIT! MAFW doesn't get the items URI!!! let's use current playlist
        // Please MAG: Fix it!!!
        else
        {
            //qDebug() << playlist->playlistName();
            QFile data(currentPlaylistUrl);
            if (data.open(QFile::ReadOnly | QFile::Truncate))
            {
                QTextStream out(&data);
                while ( !out.atEnd() )
                {
                    QString line = out.readLine();
                    QString num; num.setNum(index);
                    num.append(",localtagfs");
                    if ( line.contains(num) )
                    {
                        line.replace("%2F","/").replace("%20"," ");
                        line.remove(QRegExp("(.*)music\\/songs\\/"));
                        line.remove("music/songs/");
                        QByteArray text = QByteArray::fromPercentEncoding(line.toUtf8());
                        if ( QFileInfo(text).exists() )
                            item->setData(UserRoleSongURI, text);
                    }
                }
            }
            data.close();
        }

        /*unsigned theIndex = 0;
        int position;
        for (position = 0; position < ui->songPlaylist->count(); position++)
        {
            theIndex = ui->songPlaylist->item(position)->data(UserRoleSongIndex).toInt();
            if (theIndex > index)
                break;
        }*/

        //ui->songPlaylist->insertItem(position, item);
        this->setSongNumber(lastPlayingSong->value().toInt()+1, ui->songPlaylist->count());
        this->selectItemByText(lastPlayingSong->value().toInt());
        ui->songPlaylist->scrollToItem(ui->songPlaylist->currentItem());
    }
}

void NowPlayingWindow::onPlaylistItemActivated(QListWidgetItem *item)
{
#ifdef DEBUG
    qDebug() << "Selected item number: " << ui->songPlaylist->currentRow();
    qDebug() << "Item number in MAFW playlist: " << item->text();
#endif
    this->setSongNumber(ui->songPlaylist->currentRow()+1, ui->songPlaylist->count());
    lastPlayingSong->set(ui->songPlaylist->currentItem()->text().toInt());

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
    QTime t(0, 0);
    t = t.addSecs(item->data(UserRoleSongDuration).toInt());
    this->songDuration = item->data(UserRoleSongDuration).toInt();
    ui->trackLengthLabel->setText(t.toString("mm:ss"));
    mafwrenderer->gotoIndex(item->text().toInt());
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
#ifdef Q_WS_MAEMO_5
    QString filename = ui->songPlaylist->currentItem()->data(UserRoleSongURI).toString();
    QDBusInterface setRingtone("com.nokia.profiled",
                               "/com/nokia/profiled",
                               "com.nokia.profiled",
                               QDBusConnection::sessionBus(), this);
    setRingtone.call("set_value", "general", "ringing.alert.tone", filename);
    QMaemo5InformationBox::information(this, "Selected song set as ringing tone");
#endif
}

void NowPlayingWindow::onShareClicked()
{
#ifdef Q_WS_MAEMO_5
    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
    QStringList list;
    QString clip = ui->songPlaylist->currentItem()->data(UserRoleSongURI).toString();
#ifdef DEBUG
    qDebug() << "Sending file:" << clip;
#endif
    list.append(clip);
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
#endif
}

void NowPlayingWindow::showEntertainmentview()
{
    entertainmentView = new EntertainmentView(this, mafwFactory);
    entertainmentView->setAttribute(Qt::WA_DeleteOnClose);
    for (int i = 0; i < ui->songPlaylist->count(); i++) {
        entertainmentView->addItemToPlaylist(ui->songPlaylist->item(i));
    }
    connect(entertainmentView, SIGNAL(destroyed()), this, SLOT(nullEntertainmentView()));
    this->updateEntertainmentViewMetadata();
    entertainmentView->showFullScreen();
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

void NowPlayingWindow::nullEntertainmentView()
{
    entertainmentView = 0;
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
    playlist->removeItem(ui->songPlaylist->currentItem()->text().toInt());
#endif
    ui->songPlaylist->removeItemWidget(ui->songPlaylist->currentItem());
    delete ui->songPlaylist->currentItem();
#ifdef MAFW
    this->setSongNumber(lastPlayingSong->value().toInt(), playlist->getSize());
#endif
}

void NowPlayingWindow::selectItemByText(int numberInPlaylist)
{
    for (int i = 0; i < ui->songPlaylist->count(); i++) {
        if (ui->songPlaylist->item(i)->text().toInt() == numberInPlaylist) {
            ui->songPlaylist->clearSelection();
            ui->songPlaylist->item(i)->setSelected(true);
            ui->songPlaylist->setCurrentItem(ui->songPlaylist->item(i));
            ui->songPlaylist->scrollToItem(ui->songPlaylist->item(i));
        }
    }
}

void NowPlayingWindow::updatePlaylist()
{
    int songCount = playlist->getSize();

    // Make a new item for all items
    if (songCount != ui->songPlaylist->count()) {
        if (ui->songPlaylist->count() != 0) {
            for (int y = 0; y < ui->songPlaylist->count(); y++)
                delete ui->songPlaylist->item(y);
        }

        for (int i = 0; i < songCount; i++) {
            QListWidgetItem *item = new QListWidgetItem(ui->songPlaylist);
            ui->songPlaylist->addItem(item);
            item->setData(UserRoleValueText, " ");
            item->setData(UserRoleSongDuration, -10);
        }
    }

    // Iterate over every 30 items and fetch those
    for (int x = 0; x < songCount; x = x+30) {
        if (x > songCount) {
            playlist->getItems(x, -1);
            break;
        }
        playlist->getItems(x, x+30);
    }
}

void NowPlayingWindow::on_view_customContextMenuRequested(QPoint pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Change album art"), this, SLOT(changeArt()));
    contextMenu->exec(mapToGlobal(pos));
}

void NowPlayingWindow::changeArt()
{
    Home* hw = new Home(this, tr("Change album art"), "/home/user/MyDocs", ui->albumNameLabel->whatsThis());
    hw->exec();
    if ( hw->result() == QDialog::Accepted )
        setAlbumImage(hw->newalbumart);
    delete hw;
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
    QString file = "/home/user/.lyrics/"+ui->lyricsText->whatsThis().replace("/","-");
    QFile data(file);
    if (data.open(QFile::ReadOnly | QFile::Truncate))
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
        data->get(QNetworkRequest(QUrl("http://lyrics.mirkforce.net/" + ui->lyricsText->whatsThis())));
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
    data = data.remove(QRegExp("\\W")).remove(" ");
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
