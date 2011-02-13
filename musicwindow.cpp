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
    myNowPlayingWindow = new NowPlayingWindow(this, mafwrenderer, mafwTrackerSource);
#else
    myNowPlayingWindow = new NowPlayingWindow(this);
#endif
    ui->centralwidget->setLayout(ui->songsLayout);
    SongListItemDelegate *delegate = new SongListItemDelegate(ui->songList);
    ArtistListItemDelegate *artistDelegate = new ArtistListItemDelegate(ui->artistList);
    ui->songList->setItemDelegate(delegate);
    ui->artistList->setItemDelegate(artistDelegate);
    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);
    this->loadViewState();
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
    this->connectSignals();
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::selectSong()
{
    myNowPlayingWindow->onSongSelected(ui->songList->currentRow()+1,
                                       ui->songList->count(),
                                       ui->songList->currentItem()->data(UserRoleSongTitle).toString(),
                                       ui->songList->currentItem()->data(UserRoleSongAlbum).toString(),
                                       ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                                       ui->songList->currentItem()->data(UserRoleSongDurationS).toInt()
                                       );
    if(!ui->songList->currentItem()->data(UserRoleAlbumArt).isNull())
        myNowPlayingWindow->setAlbumImage(ui->songList->currentItem()->data(UserRoleAlbumArt).toString());
    else
        myNowPlayingWindow->setAlbumImage(albumImage);
    myNowPlayingWindow->show();
#ifdef Q_WS_MAEMO_5
    mafwrenderer->playObject(ui->songList->currentItem()->data(UserRoleObjectID).toString().toAscii());
#endif
    ui->songList->clearSelection();
}

void MusicWindow::connectSignals()
{
#ifdef Q_WS_MAEMO_5
    connect(ui->songList, SIGNAL(activated(QModelIndex)), this, SLOT(selectSong()));
#else
    connect(ui->songList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectSong()));
#endif
    connect(ui->albumList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onAlbumSelected(QListWidgetItem*)));
    connect(ui->songList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenuRequested(const QPoint &)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->indicator, SIGNAL(clicked()), myNowPlayingWindow, SLOT(show()));
}

void MusicWindow::onContextMenuRequested(const QPoint &point)
{
    contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Set as ringing tone"));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void MusicWindow::onShareClicked()
{
    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
    QStringList list;
    QString clip = ui->songList->currentItem()->data(UserRoleSongURI).toString();
    qDebug() << ui->songList->selectedItems().first()->data(UserRoleSongURI).toString();
    list.append(clip);
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
}

void MusicWindow::onDeleteClicked()
{
    QFile song(ui->songList->currentItem()->data(UserRoleSongURI).toString());
    if(song.exists()) {
        qDebug() << "Song exists";
        QMessageBox confirmDelete(QMessageBox::NoIcon,
                                  tr("Delete song?"),
                                  tr("Are you sure you want to delete this song?")+ "\n\n"
                                  + ui->songList->currentItem()->data(UserRoleSongTitle).toString() + "\n"
                                  + ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                                  QMessageBox::Yes | QMessageBox::No,
                                  this);
        confirmDelete.exec();
        if(confirmDelete.result() == QMessageBox::Yes)
            //song.remove();
            ui->songList->clear();
        else if(confirmDelete.result() == QMessageBox::No)
            ui->songList->clearSelection();
    }
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
#ifdef MAFW
    if(mafwTrackerSource->isReady())
        this->listAlbums();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listAlbums()));
#endif
}

void MusicWindow::showArtistView()
{
    this->hideLayoutContents();
    ui->artistList->show();
    this->populateMenuBar();
    QMainWindow::setWindowTitle(tr("Artists"));
    this->saveViewState("artists");
#ifdef MAFW
    if(mafwTrackerSource->isReady())
        this->listArtists();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listArtists()));
#endif
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
#ifdef MAFW
    if(mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
#endif
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

#ifdef MAFW
void MusicWindow::onAlbumSelected(QListWidgetItem *item)
{
    SingleAlbumView *albumView = new SingleAlbumView(this, this->mafwrenderer, this->mafwTrackerSource);
    albumView->setAttribute(Qt::WA_DeleteOnClose);
    albumView->browseAlbum(item->data(UserRoleSongAlbum).toString());
    albumView->show();
    ui->albumList->clearSelection();
}

void MusicWindow::listSongs()
{
#ifdef DEBUG
    qDebug() << "MusicWindow: Source ready";
#endif
    ui->songList->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllSongsId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, NULL,
                                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_ALBUM,
                                                                              MAFW_METADATA_KEY_ARTIST,
                                                                              MAFW_METADATA_KEY_URI,
                                                                              MAFW_METADATA_KEY_ALBUM_ART_URI,
                                                                              MAFW_METADATA_KEY_DURATION),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listArtists()
{
#ifdef DEBUG
    qDebug("Source ready");
#endif
    ui->artistList->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllArtists(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllArtistsId = mafwTrackerSource->sourceBrowse("localtagfs::music/artists", false, NULL, NULL,
                                                               MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                               MAFW_METADATA_KEY_DURATION,
                                                               MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                               MAFW_METADATA_KEY_CHILDCOUNT_2,
                                                               MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI),
                                                               0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listAlbums()
{
    ui->albumList->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllAlbums(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllAlbumsId = mafwTrackerSource->sourceBrowse("localtagfs::music/albums", false, NULL, NULL,
                                                              MAFW_SOURCE_LIST(MAFW_METADATA_KEY_ALBUM,
                                                                               MAFW_METADATA_KEY_ARTIST,
                                                                               MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI),
                                                              0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if(browseId != browseAllSongsId)
      return;


    QString title;
    QString artist;
    QString album;
    //QString genre("--");
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

        QListWidgetItem *item = new QListWidgetItem(ui->songList);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleObjectID, objectId);
        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleSongURI, QString::fromUtf8(filename));
            }
        }
        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleAlbumArt, QString::fromUtf8(filename));
            }
        }

        if(duration != -1) {
            QTime t(0,0);
            t = t.addSecs(duration);
            item->setData(UserRoleSongDuration, t.toString("mm:ss"));
            item->setData(UserRoleSongDurationS, duration);
        } else {
            item->setData(UserRoleSongDuration, "--:--");
            item->setData(UserRoleSongDurationS, 0);
        }
        // Although we don't need this to show the song title, we need it to
        // sort alphabatically.
        item->setText(title);
        ui->songList->addItem(item);
#ifdef Q_WS_MAEMO_5
        if(remainingCount != 0)
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
        else
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
  }
}

void MusicWindow::browseAllArtists(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if(browseId != browseAllArtistsId)
        return;

    QString title;
    int songCount = -1;
    int albumCount = -1;
    QListWidgetItem *item = new QListWidgetItem(ui->artistList);
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown artist)";
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
                item->setData(UserRoleAlbumArt, filename);
            }
        }
    }

    item->setData(UserRoleSongName, title);
    item->setData(UserRoleSongCount, songCount);
    item->setData(UserRoleAlbumCount, albumCount);
    item->setData(UserRoleObjectID, objectId);
    ui->artistList->addItem(item);
    if(!error.isEmpty())
        qDebug() << error;
#ifdef Q_WS_MAEMO_5
        if(remainingCount != 0)
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
        else
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}

void MusicWindow::browseAllAlbums(uint browseId, int remainingCount, uint, QString, GHashTable* metadata, QString error)
{
    if(browseId != browseAllAlbumsId)
        return;

    QString albumTitle;
    QString artist;
    QString albumArt;
    QListWidgetItem *item = new QListWidgetItem(ui->albumList);
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown album)";

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown artist)";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleAlbumArt, filename);
            }
        }
    }

    item->setData(UserRoleSongAlbum, albumTitle);
    item->setData(UserRoleAlbumCount, artist);
    item->setText(albumTitle);
    if(item->data(UserRoleAlbumArt).isNull())
        item->setIcon(QIcon(defaultAlbumArtMedium));
    else {
        QPixmap icon(item->data(UserRoleAlbumArt).toString());
        item->setIcon(QIcon(icon.scaled(124, 124)));
    }
    ui->artistList->addItem(item);
    if(!error.isEmpty())
        qDebug() << error;
#ifdef Q_WS_MAEMO_5
        if(remainingCount != 0)
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
        else
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}
#endif

void MusicWindow::focusInEvent(QFocusEvent *)
{
    ui->indicator->triggerAnimation();
}

void MusicWindow::focusOutEvent(QFocusEvent *)
{
    ui->indicator->stopAnimation();
}
