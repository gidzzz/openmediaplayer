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

MusicWindow::MusicWindow(QWidget *parent, MafwAdapterFactory *factory) :
        QMainWindow(parent),
#ifdef MAFW
        ui(new Ui::MusicWindow),
        mafwFactory(factory),
        mafwrenderer(factory->getRenderer()),
        mafwTrackerSource(factory->getTrackerSource()),
        playlist(factory->getPlaylistAdapter())
#else
        ui(new Ui::MusicWindow)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
#ifdef MAFW
    mafw_playlist_manager = new MafwPlaylistManagerAdapter(this);
#endif
    ui->centralwidget->setLayout(ui->songsLayout);
    ui->searchWidget->hide();
    SongListItemDelegate *delegate = new SongListItemDelegate(ui->songList);
    ArtistListItemDelegate *artistDelegate = new ArtistListItemDelegate(ui->artistList);
    ThumbnailItemDelegate *albumDelegate = new ThumbnailItemDelegate(ui->albumList);
    nowPlayingWindow = 0;

    ui->songList->setItemDelegate(delegate);
    ui->artistList->setItemDelegate(artistDelegate);
    ui->albumList->setItemDelegate(albumDelegate);
    ui->genresList->setItemDelegate(delegate);
    ui->playlistList->setItemDelegate(delegate);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->albumList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->artistList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->genresList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->playlistList->setContextMenuPolicy(Qt::CustomContextMenu);

    this->loadViewState();
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
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
    playlist->assignAudioPlaylist();
    if (nowPlayingWindow == 0) {
        nowPlayingWindow = new NowPlayingWindow(this, mafwFactory);
#else
        nowPlayingWindow = new NowPlayingWindow(this);
#endif
        nowPlayingWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(nowPlayingWindow, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        connect(nowPlayingWindow, SIGNAL(destroyed()), this, SLOT(onWindowDestroyed()));
    }
#ifdef MAFW

    playlist->clear();
    int selectedIndex = ui->songList->currentRow();
    for (int i = 0; i < ui->songList->count(); i++) {
        QListWidgetItem *item = ui->songList->item(i);
        playlist->appendItem(item->data(UserRoleObjectID).toString());
        if (ui->songList->row(item) == selectedIndex) {
            mafwrenderer->gotoIndex(selectedIndex);
            mafwrenderer->play();
            mafwrenderer->resume();
        }
    }

#endif
    nowPlayingWindow->show();
    ui->indicator->hide();
    ui->songList->clearSelection();
}

void MusicWindow::onWindowDestroyed()
{
    nowPlayingWindow = 0;
}

void MusicWindow::onPlaylistSelected(QListWidgetItem *item)
{
    if (item->data(Qt::UserRole).toBool())
        return;

    int row = ui->playlistList->row(item);
    if (row >= 1 && row <= 4) {
        SinglePlaylistView *window = new SinglePlaylistView(this, mafwFactory);
        window->setWindowTitle(item->text());
        if (row == 1)
            window->browseAutomaticPlaylist("", "-added", 30);
        else if (row == 2)
            window->browseAutomaticPlaylist("(play-count>0)", "-last-played", 30);
        else if (row == 3)
            window->browseAutomaticPlaylist("(play-count>0)", "-play-count,+title", 30);
        else if (row == 4)
            window->browseAutomaticPlaylist("(play-count=)", "", MAFW_SOURCE_BROWSE_ALL);
        window->show();
        connect(window, SIGNAL(destroyed()), this, SLOT(listPlaylists()));
    } else if (row > 5) {
        SinglePlaylistView *window = new SinglePlaylistView(this, mafwFactory);
        window->setWindowTitle(item->text());
        if (item->data(UserRoleObjectID).isNull()) // saved playlist case
            window->browsePlaylist(MAFW_PLAYLIST(mafw_playlist_manager->createPlaylist(item->text())));
        else // imported playlist case
            window->browseObjectId(item->data(UserRoleObjectID).toString());
        window->show();
        connect(window, SIGNAL(destroyed()), this, SLOT(listPlaylists()));
    }
    ui->playlistList->clearSelection();
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
    connect(ui->genresList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onGenreSelected(QListWidgetItem*)));
    connect(ui->playlistList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onPlaylistSelected(QListWidgetItem*)));
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAutomaticPlaylists(uint,int,uint,QString,GHashTable*,QString)));
#endif
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->albumList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->artistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->genresList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->playlistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));
}

void MusicWindow::onContextMenuRequested(QPoint point)
{
    QString valueText = currentList()->indexAt(point).data(UserRoleValueText).toString();
    if (this->currentList() == ui->playlistList && valueText.isEmpty()) {
        qDebug() << "suppressing context menu";
        return;
    }

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    if (this->currentList() == ui->playlistList) {
        if (ui->playlistList->currentRow() > 4 && !ui->playlistList->currentItem()->data(Qt::UserRole).toBool()) {
            if (ui->playlistList->currentItem()->data(UserRoleObjectID).isNull())
                contextMenu->addAction(tr("Delete playlist"), this, SLOT(onDeletePlaylistClicked()));
            else
                contextMenu->addAction(tr("Delete playlist"), this, SLOT(onDeleteClicked()));
        } else {
            contextMenu->exec(point);
            return;
        }
    }
    if (this->currentList() != ui->playlistList)
        contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    if (this->currentList() == ui->songList) {
        contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
        contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    }
    contextMenu->exec(point);
}

void MusicWindow::onDeletePlaylistClicked()
{
#ifdef MAFW
    if (ui->playlistList->currentItem()->data(UserRoleObjectID).isNull()) {
        QMessageBox confirmDelete(QMessageBox::NoIcon,
                                  " ",
                                  tr("Delete selected item from device?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  this);
        confirmDelete.exec();
        if (confirmDelete.result() == QMessageBox::Yes) {
            mafw_playlist_manager->deletePlaylist(ui->playlistList->currentItem()->text());
            delete ui->playlistList->currentItem();
        }
        else if (confirmDelete.result() == QMessageBox::No)
            ui->playlistList->clearSelection();
    }
#endif
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

    if (objectId != this->currentList()->currentItem()->data(UserRoleObjectID).toString())
        return;

    QFile song(uri);
    if(song.exists()) {
        if (this->currentList() == ui->songList) {
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
                delete ui->songList->currentItem();
            }
            else if(confirmDelete.result() == QMessageBox::No)
                ui->songList->clearSelection();
        } else if (this->currentList() == ui->playlistList) {
            QMessageBox confirmDelete(QMessageBox::NoIcon,
                                      " ",
                                      tr("Delete selected item from device?"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      this);
            confirmDelete.exec();
            if(confirmDelete.result() == QMessageBox::Yes) {
                song.remove();
                ui->playlistList->removeItemWidget(ui->playlistList->currentItem());
                delete ui->playlistList->currentItem();
            }
            else if(confirmDelete.result() == QMessageBox::No)
                ui->playlistList->clearSelection();
        }
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
#ifdef MAFW
    if (ui->genresList->count() == 0) {
        if (mafwTrackerSource->isReady())
            this->listGenres();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listGenres()));
    }
#endif
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
#ifdef MAFW
    if (mafwTrackerSource->isReady())
        this->listPlaylists();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listPlaylists()));
#endif
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
    SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
    albumView->setAttribute(Qt::WA_DeleteOnClose);
    albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
    albumView->setWindowTitle(item->data(Qt::DisplayRole).toString());
    connect(albumView, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
    albumView->show();
    ui->indicator->hide();
    ui->albumList->clearSelection();
}

void MusicWindow::onArtistSelected(QListWidgetItem *item)
{
    int songCount = item->data(UserRoleAlbumCount).toInt();
    if(songCount == 0 || songCount == 1) {
        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        if (songCount == 1)
            albumView->isSingleAlbum = true;
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setAttribute(Qt::WA_DeleteOnClose);
        albumView->setWindowTitle(item->data(UserRoleSongName).toString());
        connect(albumView, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        albumView->show();
        ui->indicator->hide();
    } else if(songCount > 1) {
        SingleArtistView *artistView = new SingleArtistView(this, mafwFactory);
        artistView->browseAlbum(item->data(UserRoleObjectID).toString());
        artistView->setWindowTitle(item->data(UserRoleSongName).toString());
        artistView->setSongCount(item->data(UserRoleSongCount).toInt());
        artistView->setAttribute(Qt::WA_DeleteOnClose);
        connect(artistView, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
        artistView->show();
        ui->indicator->hide();
    }

    ui->artistList->clearSelection();
}

void MusicWindow::onGenreSelected(QListWidgetItem *item)
{
    SingleGenreView *genreWindow = new SingleGenreView(this, mafwFactory);
    genreWindow->setAttribute(Qt::WA_DeleteOnClose);
    genreWindow->setWindowTitle(item->data(UserRoleSongTitle).toString());
    connect(genreWindow, SIGNAL(destroyed()), ui->indicator, SLOT(show()));
    genreWindow->show();
    ui->indicator->hide();
    genreWindow->setSongCount(item->data(UserRoleSongCount).toInt());
    genreWindow->browseGenre(item->data(UserRoleObjectID).toString());

    ui->genresList->clearSelection();
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

void MusicWindow::listGenres()
{
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllGenres(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllGenresId = mafwTrackerSource->sourceBrowse("localtagfs::music/genres", false, NULL, NULL,
                                                              MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                               MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                               MAFW_METADATA_KEY_CHILDCOUNT_2,
                                                                               MAFW_METADATA_KEY_CHILDCOUNT_3),
                                                              0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listPlaylists()
{
    for (int x = 0; x < ui->playlistList->count(); x++) {
        ui->playlistList->removeItemWidget(ui->playlistList->item(x));
        delete ui->playlistList->item(x);
        ui->playlistList->clear();
    }

    recentlyAddedCount = 0;
    recentlyPlayedCount = 0;
    mostPlayedCount = 0;
    neverPlayedCount = 0;

    this->listAutoPlaylists();

    GArray* playlists = mafw_playlist_manager->listPlaylists();
    QString playlistName;

    if (playlists->len != 0) {
        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setText(tr("Saved"));
        listItem->setData(Qt::UserRole, true);
        ui->playlistList->addItem(listItem);
    }

    for (uint i = 0; i < playlists->len; i++) {
        MafwPlaylistManagerItem* item = &g_array_index(playlists, MafwPlaylistManagerItem, i);

        playlistName = QString::fromUtf8(item->name);
        int playlistCount = playlist->getSizeOf(MAFW_PLAYLIST (mafw_playlist_manager->getPlaylist(item->id)));

        if (playlistName != "FmpAudioPlaylist" && playlistName != "FmpVideoPlaylist" && playlistName != "FmpRadioPlaylist"
            && playlistCount != 0) {
            QListWidgetItem *listItem = new QListWidgetItem();
            listItem->setText(playlistName);
            listItem->setData(UserRoleSongCount, playlistCount);
            QString valueText = QString::number(playlistCount) + " ";

            if ( playlistCount > 1 ) listItem->setData(Qt::UserRole+50, tr("songs"));
            else listItem->setData(Qt::UserRole+50, tr("song"));

            if (playlistCount == 1)
                valueText.append(tr("song"));
            else
                valueText.append(tr("songs"));
            listItem->setData(UserRoleValueText, valueText);
            ui->playlistList->addItem(listItem);
        }
    }
    mafw_playlist_manager_free_list_of_playlists(playlists);

    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::UserRole, true);
    item->setText(tr("Imported playlists"));
    ui->playlistList->addItem(item);

    this->listImportedPlaylists();
}

void MusicWindow::listAutoPlaylists()
{
    QListWidgetItem *listItem = new QListWidgetItem();
    listItem->setText(tr("Automatic playlists"));
    listItem->setData(Qt::UserRole, true);
    ui->playlistList->insertItem(0, listItem);

    QStringList playlists;
    playlists << tr("Recently added") << tr("Recently played") << tr("Most played") << tr("Never played");
    foreach (QString string, playlists) {
        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setText(string);
        ui->playlistList->addItem(listItem);
    }

    this->browseNeverPlayedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false,
                                                                "(play-count=)",
                                                                NULL,
                                                                MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                                 MAFW_METADATA_KEY_DURATION),
                                                                0, MAFW_SOURCE_BROWSE_ALL);

    this->browseMostPlayedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false,
                                                               "(play-count>0)",
                                                               "-play-count,+title",
                                                               MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                                MAFW_METADATA_KEY_DURATION),
                                                               0, 30);

    this->browseRecentlyPlayedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false,
                                                                   "(play-count>0)",
                                                                   "-last-played",
                                                                   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                                    MAFW_METADATA_KEY_DURATION),
                                                                   0, 30);

    this->browseRecentlyAddedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, "-added",
                                                                  MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                                   MAFW_METADATA_KEY_DURATION),
                                                                  0, 30);
}

void MusicWindow::listImportedPlaylists()
{
    browseImportedPlaylistsId = mafwTrackerSource->sourceBrowse("localtagfs::music/playlists", false, NULL, NULL,
                                                                MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                                 MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                                 MAFW_METADATA_KEY_DURATION),
                                                                0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::browseAutomaticPlaylists(uint browseId, int, uint, QString objectId, GHashTable *metadata, QString) // not really, imported playlists too
{
    GValue *v;
    if (browseId == this->browseRecentlyAddedId) {
        recentlyAddedCount++;
        QString valueText = QString::number(recentlyAddedCount) + " ";
        if (recentlyAddedCount == 1)
            valueText.append(tr("song"));
        else
            valueText.append(tr("songs"));
        ui->playlistList->item(1)->setData(UserRoleValueText, valueText);
    } else if (browseId == this->browseRecentlyPlayedId) {
        recentlyPlayedCount++;
        QString valueText = QString::number(recentlyPlayedCount) + " ";
        if (recentlyPlayedCount == 1)
            valueText.append(tr("song"));
        else
            valueText.append(tr("songs"));
        ui->playlistList->item(2)->setData(UserRoleValueText, valueText);
    } else if (browseId == this->browseMostPlayedId) {
        mostPlayedCount++;
        QString valueText = QString::number(mostPlayedCount) + " ";
        if (mostPlayedCount == 1)
            valueText.append(tr("song"));
        else
            valueText.append(tr("songs"));
        ui->playlistList->item(3)->setData(UserRoleValueText, valueText);
    } else if (browseId == this->browseNeverPlayedId) {
        neverPlayedCount++;
        QString valueText = QString::number(neverPlayedCount) + " ";
        if (neverPlayedCount == 1)
            valueText.append(tr("song"));
        else
            valueText.append(tr("songs"));
        ui->playlistList->item(4)->setData(UserRoleValueText, valueText);
    } else if (browseId == this->browseImportedPlaylistsId) {
        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setData(UserRoleObjectID, objectId);
        v = mafw_metadata_first (metadata,
                                 MAFW_METADATA_KEY_TITLE);
        listItem->setText(g_value_get_string (v));

        v = mafw_metadata_first (metadata,
                                 MAFW_METADATA_KEY_CHILDCOUNT_1);

        int songCount = g_value_get_int (v);

        QString valueText = QString::number(songCount) + " ";
        if (songCount == 1)
            valueText.append(tr("song"));
        else
            valueText.append(tr("songs"));
        listItem->setData(UserRoleValueText, valueText);
        ui->playlistList->addItem(listItem);
    }
    ui->playlistList->scroll(0,0);
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

    if ( songCount > 1 ) item->setData(Qt::UserRole+50, tr("songs"));
    else item->setData(Qt::UserRole+50, tr("song"));
    if ( albumCount > 1 ) item->setData(Qt::UserRole+51, tr("albums"));
    else item->setData(Qt::UserRole+51, tr("album"));

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

    if ( songCount > 1 ) item->setData(Qt::UserRole+50, tr("songs"));
    else item->setData(Qt::UserRole+50, tr("song"));

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

void MusicWindow::browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable *metadata, QString error)
{
    if (this->browseAllGenresId != browseId)
        return;

    QString title;
    QString valueText;
    int songCount = -1;
    int albumCount = -1;
    int artistCount = -1;
    GValue *v;

    v = mafw_metadata_first (metadata,
                             MAFW_METADATA_KEY_TITLE);
    title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown genre)");

    v = mafw_metadata_first (metadata,
                             MAFW_METADATA_KEY_CHILDCOUNT_1);
    artistCount = v ? g_value_get_int (v) : -1;

    v = mafw_metadata_first (metadata,
                             MAFW_METADATA_KEY_CHILDCOUNT_2);
    albumCount = v ? g_value_get_int (v) : -1;

    v = mafw_metadata_first (metadata,
                             MAFW_METADATA_KEY_CHILDCOUNT_3);
    songCount = v ? g_value_get_int (v) : -1;

    QListWidgetItem *item = new QListWidgetItem();
    if (!title.isEmpty())
        item->setData(UserRoleSongTitle, title);
    else
        item->setData(UserRoleSongTitle, tr("(unknown genre)"));
    item->setText(item->data(UserRoleSongTitle).toString());
    item->setData(UserRoleSongCount, songCount);
    item->setData(UserRoleArtistCount, artistCount);
    item->setData(UserRoleAlbumCount, albumCount);
    item->setData(UserRoleObjectID, objectId);

    if ( songCount > 1 ) item->setData(Qt::UserRole+50, tr("songs"));
    else item->setData(Qt::UserRole+50, tr("song"));
    if ( albumCount > 1 ) item->setData(Qt::UserRole+51, tr("albums"));
    else item->setData(Qt::UserRole+51, tr("album"));

    valueText.append(QString::number(songCount));
    valueText.append(" ");

    if (songCount == 1)
        valueText.append(tr("song"));
    else
        valueText.append(tr("songs"));

    valueText.append(", ");

    valueText.append(QString::number(albumCount));
    valueText.append(" ");

    if (albumCount == 1)
        valueText.append(tr("album"));
    else
        valueText.append(tr("albums"));

    valueText.append(", ");

    valueText.append(QString::number(artistCount));
    valueText.append(" ");

    if (artistCount == 1)
        valueText.append(tr("artist"));
    else
        valueText.append(tr("artists"));

    item->setData(UserRoleValueText, valueText);

    ui->genresList->addItem(item);

    if(!error.isEmpty())
        qDebug() << error;

#ifdef Q_WS_MAEMO_5
        if (remainingCount != 0)
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
#ifdef Q_WS_MAEMO_5
        this->notifyOnAddedToNowPlaying(1);
#endif
    }
    else if (this->currentList() == ui->artistList || this->currentList() == ui->albumList || this->currentList() == ui->genresList) {
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
    else if (this->currentList() == ui->playlistList) {
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
        QListWidgetItem *item = this->currentList()->currentItem();

        qDebug() << "item id: " << item->data(UserRoleObjectID).toString();

        int row = this->currentList()->row(item);
        if (row < 5) { // automatic playlist case
            QString filter;
            QString sorting;
            int limit = 30; // TODO: user-configurable?
            switch (row) {
                case 1: filter = ""; sorting = "-added"; break;
                case 2: filter = "(play-count>0)"; sorting = "-last-played"; break;
                case 3: filter = "(play-count>0)"; sorting = "-play-count,+title"; break;
                case 4: filter = "(play-count=)"; sorting = "";
            }
            qDebug() << "connecting MusicWindow to signalSourceBrowseResult";
            connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

            this->addToNowPlayingId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", true, filter.toUtf8(), sorting.toUtf8(),
                                                                     MAFW_SOURCE_NO_KEYS, 0, limit);
        }
        else if (item->data(UserRoleObjectID).isNull()) { // saved playlist case
            MafwPlaylist *mafwplaylist = MAFW_PLAYLIST(mafw_playlist_manager->createPlaylist(item->text()));
            songAddBufferSize = numberOfSongsToAdd = playlist->getSizeOf(mafwplaylist);

            if (numberOfSongsToAdd > 0) {
                songAddBuffer = new QString[songAddBufferSize];
                qDebug() << "connecting MusicWindow to onGetItems";
                connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint)),
                    this, SLOT(onGetItems(QString,GHashTable*,guint)));
                playlist->getItemsOf(mafwplaylist);
            }
        }
        else { // imported playlist case
            QString objectId = item->data(UserRoleObjectID).toString();

            qDebug() << "connecting MusicWindow to signalSourceBrowseResult";
            connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

            this->addToNowPlayingId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                                      MAFW_SOURCE_NO_KEYS, 0, MAFW_SOURCE_BROWSE_ALL);
        }
    }
#endif
}

void MusicWindow::onGetItems(QString objectId, GHashTable*, guint index)
{
    qDebug() << "MusicWindow::onGetItems(QString, GHashTable*, guint) | index: " << index;
    songAddBuffer[index] = objectId;
    numberOfSongsToAdd--;

    if (numberOfSongsToAdd == 0) {
        this->notifyOnAddedToNowPlaying(songAddBufferSize);

        qDebug() << "disconnecting MusicWindow from onGetItems";
        disconnect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint)),
            this, SLOT(onGetItems(QString,GHashTable*,guint)));

        for (int i = 0; i < songAddBufferSize; i++)
            playlist->appendItem(songAddBuffer[i]);

        delete[] songAddBuffer;
        songAddBufferSize = 0;
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void MusicWindow::onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString)
{
    if (browseId != this->addToNowPlayingId)
        return;

    qDebug() << "MusicWindow::onGetItems(QString, GHashTable*, guint) | index: " << index;
    playlist->appendItem(objectId); // when items seem to be delivered in correct order, appending is sufficient

    if (remainingCount == 0) {
        this->notifyOnAddedToNowPlaying(index+1); // when items are delivered in correct order, index determines operation size
        qDebug() << "disconnecting MusicWindow from signalSourceBrowseResult";
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));
        this->addToNowPlayingId = 0;
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
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

void MusicWindow::showEvent(QShowEvent *)
{
    emit shown();
}

void MusicWindow::hideEvent(QHideEvent *)
{
    emit hidden();
}
