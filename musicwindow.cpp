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

#include "musicwindow.h"

MusicWindow::MusicWindow(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa, MafwPlaylistAdapter* pls) :
        QMainWindow(parent),
#ifdef MAFW
        ui(new Ui::MusicWindow),
        mafwrenderer(mra),
        mafwTrackerSource(msa),
        playlist(pls)
#else
        ui(new Ui::MusicWindow)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
    ui->centralwidget->setLayout(ui->songsLayout);
    ui->searchWidget->hide();
    SongListItemDelegate *delegate = new SongListItemDelegate(ui->songList);
    ArtistListItemDelegate *artistDelegate = new ArtistListItemDelegate(ui->artistList);
    ThumbnailItemDelegate *albumDelegate = new ThumbnailItemDelegate(ui->albumList);
    ui->songList->setItemDelegate(delegate);
    ui->artistList->setItemDelegate(artistDelegate);
    ui->albumList->setItemDelegate(albumDelegate);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->albumList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->artistList->setContextMenuPolicy(Qt::CustomContextMenu);

    this->loadViewState();
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
#ifdef MAFW
    ui->indicator->setSources(this->mafwrenderer, this->mafwTrackerSource, this->playlist);
#endif
    this->connectSignals();
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::onSongSelected(QListWidgetItem *)
{
#ifdef MAFW
    myNowPlayingWindow = new NowPlayingWindow(this, mafwrenderer, mafwTrackerSource, playlist);
#else
    myNowPlayingWindow = new NowPlayingWindow(this);
#endif
    myNowPlayingWindow->setAttribute(Qt::WA_DeleteOnClose);
#ifdef MAFW
    qDebug() << "Clearing playlist";
    playlist->assignAudioPlaylist();
    playlist->clear();
    qDebug() << "Playlist cleared";
    for (int i = 0; i < ui->songList->count(); i++) {
        QListWidgetItem *item = ui->songList->item(i);
        playlist->appendItem(item->data(UserRoleObjectID).toString());
    }
    qDebug() << "Playlist created";

    mafwrenderer->gotoIndex(ui->songList->currentRow());
    mafwrenderer->play();

#endif
    myNowPlayingWindow->show();
    ui->songList->clearSelection();
}

void MusicWindow::connectSignals()
{
#ifdef Q_WS_MAEMO_5
    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onSongSelected(QListWidgetItem*)));
#else
    connect(ui->songList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onSongSelected(QListWidgetItem*)));
#endif
#ifdef MAFW
    connect(ui->albumList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onAlbumSelected(QListWidgetItem*)));
    connect(ui->artistList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onArtistSelected(QListWidgetItem*)));
#endif
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->albumList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->artistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));
}

void MusicWindow::onContextMenuRequested(QPoint point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    if (this->currentList() == ui->songList) {
        contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
        contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    }
    contextMenu->exec(point);
}

void MusicWindow::setRingingTone()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void MusicWindow::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QDBusInterface setRingtone("com.nokia.profiled",
                               "/com/nokia/profiled",
                               "com.nokia.profiled",
                               QDBusConnection::sessionBus(), this);
    setRingtone.call("set_value", "general", "ringing.alert.tone", uri);
    QMaemo5InformationBox::information(this, "Selected song set as ringing tone");
}
#endif

void MusicWindow::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void MusicWindow::onShareUriReceived(QString objectId, QString Uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
    QStringList list;
    QString clip;
    clip = Uri;

    list.append(clip);
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
}
#endif

void MusicWindow::onDeleteClicked()
{
#ifdef MAFW
    this->mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onDeleteUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void MusicWindow::onDeleteUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onDeleteUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QFile song(uri);
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
        if(confirmDelete.result() == QMessageBox::Yes) {
            song.remove();
            ui->songList->removeItemWidget(ui->songList->currentItem());
        }
        else if(confirmDelete.result() == QMessageBox::No)
            ui->songList->clearSelection();
    }
}
#endif

void MusicWindow::onSearchTextChanged(QString text)
{
    if (!ui->indicator->isHidden())
        ui->indicator->hide();

    for (int i=0; i < this->currentList()->count(); i++) {
        if (this->currentList() == ui->songList) {
             if (ui->songList->item(i)->text().toLower().indexOf(text.toLower()) != -1 ||
                 ui->songList->item(i)->data(UserRoleSongArtist).toString().toLower().indexOf(text.toLower()) != -1 ||
                 ui->songList->item(i)->data(UserRoleSongAlbum).toString().toLower().indexOf(text.toLower()) != -1)
                 ui->songList->item(i)->setHidden(false);
             else
                 ui->songList->item(i)->setHidden(true);
        } else if (this->currentList() == ui->albumList) {
             if (ui->albumList->item(i)->text().toLower().indexOf(text.toLower()) != -1 ||
                 ui->albumList->item(i)->data(UserRoleValueText).toString().toLower().indexOf(text.toLower()) != -1)
                 ui->albumList->item(i)->setHidden(false);
             else
                 ui->albumList->item(i)->setHidden(true);
        } else {
            if (this->currentList()->item(i)->text().toLower().indexOf(text.toLower()) == -1)
                this->currentList()->item(i)->setHidden(true);
            else
                this->currentList()->item(i)->setHidden(false);
        }
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        if (ui->indicator->isHidden())
            ui->indicator->show();
    }
}

void MusicWindow::orientationChanged()
{
    this->currentList()->scroll(1,1);
    ui->albumList->setFlow(ui->albumList->flow());
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
    if (ui->albumList->count() == 0) {
        if(mafwTrackerSource->isReady())
            this->listAlbums();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listAlbums()));
    }
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
    if (ui->artistList->count() == 0) {
        if(mafwTrackerSource->isReady())
            this->listArtists();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listArtists()));
    }
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
    if (ui->songList->count() == 0) {
        if(mafwTrackerSource->isReady())
            this->listSongs();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
    }
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

QListWidget* MusicWindow::currentList()
{
    if (!ui->songList->isHidden())
        return ui->songList;
    else if (!ui->artistList->isHidden())
        return ui->artistList;
    else if (!ui->genresList->isHidden())
        return ui->genresList;
    else if (!ui->albumList->isHidden())
        return ui->albumList;
    else if (!ui->playlistList->isHidden())
        return ui->playlistList;
    else
        return 0;
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
    SingleAlbumView *albumView = new SingleAlbumView(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
    albumView->setAttribute(Qt::WA_DeleteOnClose);
    albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
    albumView->setWindowTitle(item->data(Qt::DisplayRole).toString());
    albumView->show();
    ui->albumList->clearSelection();
}

void MusicWindow::onArtistSelected(QListWidgetItem *item)
{
    int songCount = item->data(UserRoleAlbumCount).toInt();
    if(songCount == 0 || songCount == 1) {
        SingleAlbumView *albumView = new SingleAlbumView(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
        if (songCount == 1)
            albumView->isSingleAlbum = true;
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setAttribute(Qt::WA_DeleteOnClose);
        albumView->setWindowTitle(item->data(UserRoleSongName).toString());
        albumView->show();
    } else if(songCount > 1) {
        SingleArtistView *artistView = new SingleArtistView(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
        artistView->browseAlbum(item->data(UserRoleObjectID).toString());
        artistView->setWindowTitle(item->data(UserRoleSongName).toString());
        artistView->setSongCount(item->data(UserRoleSongCount).toInt());
        artistView->setAttribute(Qt::WA_DeleteOnClose);
        artistView->show();
    }

    ui->artistList->clearSelection();
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
                                                                               MAFW_METADATA_KEY_CHILDCOUNT_1,
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


    if (title.isEmpty())
        title = tr("(unknown artist)");

    item->setText(title);
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


void MusicWindow::browseAllAlbums(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if(browseId != browseAllAlbumsId)
        return;

    QString albumTitle;
    QString artist;
    QString albumArt;
    int songCount = -1;
    QListWidgetItem *item = new QListWidgetItem();
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown album)";

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown artist)";

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_CHILDCOUNT_1);
        songCount = v ? g_value_get_int(v) : -1;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setIcon(QIcon(QString::fromUtf8(filename)));
            }
        } else {
            item->setIcon(QIcon(defaultAlbumArtMedium));
        }
    }

    if (artist != "__VV__")
        item->setData(UserRoleValueText, artist);
    else
        item->setData(UserRoleValueText, tr("Various artists"));
    item->setData(UserRoleObjectID, objectId);
    item->setData(UserRoleSongCount, songCount);
    item->setText(albumTitle);

    ui->albumList->addItem(item);
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

void MusicWindow::keyPressEvent(QKeyEvent *)
{

}

void MusicWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
        this->currentList()->setFocus();
    else {
        this->currentList()->clearSelection();
        if (ui->searchWidget->isHidden())
            ui->searchWidget->show();
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
}

void MusicWindow::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    if (this->currentList() == ui->songList) {
        playlist->appendItem(ui->songList->currentItem()->data(UserRoleObjectID).toString());
        this->notifyOnAddedToNowPlaying(1);
    }
    else if (this->currentList() == ui->artistList || this->currentList() == ui->albumList) {
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

        QString objectIdToBrowse = this->currentList()->currentItem()->data(UserRoleObjectID).toString();
        numberOfSongsToAdd = this->currentList()->currentItem()->data(UserRoleSongCount).toInt();
        this->addToNowPlayingId = mafwTrackerSource->sourceBrowse(objectIdToBrowse.toUtf8(), true, NULL, NULL, 0,
                                                                  0, MAFW_SOURCE_BROWSE_ALL);
        connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                this, SLOT(onAddToNowPlayingCallback(uint,int,uint,QString,GHashTable*,QString)));
    }
#endif
}

#ifdef MAFW
void MusicWindow::onAddToNowPlayingCallback(uint browseId, int remainingCount, uint, QString objectId, GHashTable*, QString)
{
    if (this->addToNowPlayingId != browseId)
        return;

    playlist->appendItem(objectId);

#ifdef Q_WS_MAEMO_5
    if (remainingCount != 0)
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
    else {
       this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
       this->notifyOnAddedToNowPlaying(numberOfSongsToAdd);
   }
#endif

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onAddToNowPlayingCallback(uint,int,uint,QString,GHashTable*,QString)));
        this->addToNowPlayingId = 0;
        this->numberOfSongsToAdd = 0;
    }
}
#endif

#ifdef Q_WS_MAEMO_5
void MusicWindow::notifyOnAddedToNowPlaying(int songCount)
{
        QString addedToNp;
        if (songCount == 1)
            addedToNp = tr("clip added to now playing");
        else
            addedToNp = tr("clips added to now playing");
        QMaemo5InformationBox::information(this, QString::number(songCount) + " " + addedToNp);
}
#endif
