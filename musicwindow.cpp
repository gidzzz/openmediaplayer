#include "musicwindow.h"

MusicWindow::MusicWindow(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa) :
        QMainWindow(parent),
#ifdef Q_WS_MAEMO_5
        ui(new Ui::MusicWindow),
        mafwrenderer(mra),
        mafwTrackerSource(msa)
#else
        ui(new Ui::MusicWindow)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(this);
#else
    shuffleAllButton = new QPushButton(this);
#endif
    shuffleAllButton->setText(tr("Shuffle songs"));
    ui->songsLayout->removeWidget(ui->songList);
    ui->songsLayout->addWidget(shuffleAllButton);
    ui->songsLayout->addWidget(ui->songList);
    ui->centralwidget->setLayout(ui->songsLayout);
    myNowPlayingWindow = new NowPlayingWindow(this, mafwrenderer);
    SongListItemDelegate *delegate = new SongListItemDelegate(ui->songList);
    ArtistListItemDelegate *artistDelegate = new ArtistListItemDelegate(ui->artistList);
    ui->songList->setItemDelegate(delegate);
    ui->artistList->setItemDelegate(artistDelegate);
    //this->listSongs();
    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);
    this->connectSignals();
    if(mafwTrackerSource->isReady())
        this->trackerSourceReady();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(trackerSourceReady()));
    this->loadViewState();
#ifdef Q_WS_MAEMO_5
    int numberOfSongs = ui->songList->count();
    if(numberOfSongs == 1) {
        QString label = tr("song");
        shuffleAllButton->setValueText(QString::number(numberOfSongs) + " " + label);
    } else {
        QString label = tr("songs");
        shuffleAllButton->setValueText(QString::number(numberOfSongs) + " " + label);
    }
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    shuffleAllButton->setIconSize(QSize(64, 64));
    shuffleAllButton->setIcon(QIcon(shuffleButtonIcon));
#endif
    shuffleAllButton->hide();
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::selectSong()
{
    myNowPlayingWindow->onMetadataChanged(ui->songList->currentRow()+1,
                                          ui->songList->count(),
                                          //QString("Song name"),
                                          QString(ui->songList->currentItem()->text()),
                                          tr("(unknown album)"),
                                          tr("(unknown arist)")
                                          );
    myNowPlayingWindow->show();
    ui->songList->clearSelection();
}

void MusicWindow::listSongs()
{
    QDirIterator directory_walker(
#ifdef Q_WS_MAEMO_5
            "/home/user/MyDocs/.sounds",
#else
            "/home",
#endif
            QDir::Files | QDir::NoSymLinks,
            QDirIterator::Subdirectories);

    while(directory_walker.hasNext())
    {
        directory_walker.next();

        if(directory_walker.fileInfo().completeSuffix() == "mp3") {
            QListWidgetItem *songItem = new QListWidgetItem(directory_walker.fileName());
            songItem->setData(UserRoleSongName, directory_walker.fileName());
            //ui->songList->addItem(directory_walker.fileName());
            ui->songList->addItem(songItem);
            myNowPlayingWindow->listSongs(directory_walker.fileName());
        }
    }
}

void MusicWindow::connectSignals()
{
#ifdef Q_WS_MAEMO_5
    connect(ui->songList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectSong()));
#else
    connect(ui->songList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectSong()));
#endif
    connect(ui->songList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenuRequested(const QPoint &)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->indicator, SIGNAL(clicked()), myNowPlayingWindow, SLOT(show()));
}

void MusicWindow::onContextMenuRequested(const QPoint &point)
{
    contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"));
    contextMenu->addAction(tr("Delete"));
    contextMenu->addAction(tr("Set as ringing tone"));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void MusicWindow::onShareClicked()
{
    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
    QStringList list;
    QString clip = "/home/user/MyDocs/.sounds/" + ui->songList->statusTip();
    list.append(clip);
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
}

void MusicWindow::orientationChanged()
{
    ui->songList->scroll(1,1);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55),
                               ui->indicator->width(),ui->indicator->height());
    ui->indicator->raise();
}

void MusicWindow::populateMenuBar()
{
    ui->menubar->clear();
    if(ui->albumList->isHidden())
        ui->menubar->addAction(tr("All albums"), this, SLOT(showAlbumView()));
    if(ui->artistList->isHidden())
        ui->menubar->addAction(tr("Artists"), this, SLOT(showArtistView()));
    if(ui->songList->isHidden())
        ui->menubar->addAction(tr("All songs"), this, SLOT(showSongsView()));
    if(ui->genresList->isHidden())
        ui->menubar->addAction(tr("Genres"), this, SLOT(showGenresView()));
    if(ui->playlistList->isHidden())
        ui->menubar->addAction(tr("Playlists"), this, SLOT(showPlayListView()));
}

void MusicWindow::hideLayoutContents()
{
    if(!ui->songList->isHidden())
        ui->songList->hide();
    if(!ui->artistList->isHidden())
        ui->artistList->hide();
    if(!ui->genresList->isHidden())
        ui->genresList->hide();
    if(!ui->albumList->isHidden())
        ui->albumList->hide();
    if(!ui->playlistList->isHidden())
        ui->playlistList->hide();
}

void MusicWindow::showAlbumView()
{
    this->hideLayoutContents();
    ui->albumList->show();
    this->populateMenuBar();
    QMainWindow::setWindowTitle(tr("Albums"));
    this->saveViewState("albums");
}

void MusicWindow::showArtistView()
{
    this->hideLayoutContents();
    ui->artistList->show();
    this->populateMenuBar();
    QMainWindow::setWindowTitle(tr("Artists"));
    this->saveViewState("artists");
}

void MusicWindow::showGenresView()
{
    this->hideLayoutContents();
    ui->genresList->show();
    this->populateMenuBar();
    QMainWindow::setWindowTitle(tr("Genres"));
    this->saveViewState("genres");
}

void MusicWindow::showSongsView()
{
    this->hideLayoutContents();
    ui->songList->show();
    this->populateMenuBar();
    QMainWindow::setWindowTitle(tr("Songs"));
    this->saveViewState("songs");
}

void MusicWindow::showPlayListView()
{
    this->hideLayoutContents();
    ui->playlistList->show();
    this->populateMenuBar();
    QMainWindow::setWindowTitle(tr("Playlists"));
    this->saveViewState("playlists");
}

void MusicWindow::saveViewState(QVariant view)
{
    QSettings().setValue("music/view", view);
}

void MusicWindow::loadViewState()
{
    if(QSettings().contains("music/view")) {
        QString state = QSettings().value("music/view").toString();
        if(state == "songs")
            this->showSongsView();
        else if(state == "albums")
            this->showAlbumView();
        else if(state == "artists")
            this->showArtistView();
        else if(state == "genres")
            this->showGenresView();
        else if(state == "playlists")
            this->showPlayListView();
    } else {
        this->showSongsView();
    }
}

void MusicWindow::trackerSourceReady()
{
#ifdef DEBUG
    qDebug() << "MusicWindow: Source ready";
#endif
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllSongsId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, NULL,
                                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_ALBUM,
                                                                              MAFW_METADATA_KEY_ARTIST,
                                                                              MAFW_METADATA_KEY_DURATION),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::browseAllSongs(uint browseId, int, uint, QString, GHashTable* metadata, QString)
{
    if(browseId != browseAllSongsId)
      return;


    QString title("--");
    QString artist("--");
    QString album("--");
    //QString genre("--");
    int duration = -1;
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? g_value_get_string (v) : tr("(unknown song)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? g_value_get_string (v) : tr("(unknown artist)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        album = v ? g_value_get_string (v) : tr("(unknown album)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : -1;

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(UserRoleSongTitle, title.toAscii());
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        if(duration != -1) {
            QTime t(0,0);
            t = t.addSecs(duration);
            item->setData(UserRoleSongDuration, t.toString("mm:ss"));
        } else {
            item->setData(UserRoleSongDuration, "--:--");
        }
        // Although we don't need this to show the song title, we need it to
        // sort alphabatically.
        item->setText(title);
        ui->songList->addItem(item);
  }
}

void MusicWindow::browseAllArtists(uint browseId, int, uint, QString objectId, GHashTable* metadata, QString error)
{
    if(browseId != browseAllArtistsId)
        return;

    QString title = QString::fromUtf8("-");
    int duration = -1;
    int songCount = -1;
    int albumCount = -1;
    QListWidgetItem *item = new QListWidgetItem();
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string(v)) : "--";
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_CHILDCOUNT_1);
        albumCount = v ? g_value_get_int (v) : -1;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_CHILDCOUNT_2);
        songCount = v ? g_value_get_int (v) : -1;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(Qt::DecorationRole, QPixmap(filename));
            }
        }
    }

    item->setData(UserRoleSongTitle, title);
    item->setData(UserRoleSongDuration, duration);
    item->setData(UserRoleSongCount, songCount);
    item->setData(UserRoleAlbumCount, albumCount);
    ui->artistList->addItem(item);
    if(!error.isEmpty())
        qDebug() << error;
}
