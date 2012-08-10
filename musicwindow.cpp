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
    mafwPlaylistManager = new MafwPlaylistManagerAdapter(this);
#endif
    ui->centralwidget->setLayout(ui->songsLayout);
    SongListItemDelegate *songDelegate = new SongListItemDelegate(ui->songList);
    ArtistListItemDelegate *artistDelegate = new ArtistListItemDelegate(ui->artistList);
    ThumbnailItemDelegate *albumDelegate = new ThumbnailItemDelegate(ui->albumList);

    ui->songList->setItemDelegate(songDelegate);
    ui->artistList->setItemDelegate(artistDelegate);
    ui->albumList->setItemDelegate(albumDelegate);
    ui->genresList->setItemDelegate(songDelegate);
    ui->playlistList->setItemDelegate(songDelegate);

    ui->songList->viewport()->installEventFilter(this);
    ui->albumList->viewport()->installEventFilter(this);
    ui->artistList->viewport()->installEventFilter(this);
    ui->genresList->viewport()->installEventFilter(this);
    ui->playlistList->viewport()->installEventFilter(this);

    songModel = new QStandardItemModel(this);
    songProxyModel = new QSortFilterProxyModel(this);
    songProxyModel->setFilterRole(UserRoleFilterString);
    songProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    songProxyModel->setSourceModel(songModel);
    ui->songList->setModel(songProxyModel);

    albumModel = new QStandardItemModel(this);
    albumProxyModel = new QSortFilterProxyModel(this);
    albumProxyModel->setFilterRole(UserRoleFilterString);
    albumProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    albumProxyModel->setSourceModel(albumModel);
    ui->albumList->setModel(albumProxyModel);

    artistModel = new QStandardItemModel(this);
    artistProxyModel = new QSortFilterProxyModel(this);
    artistProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    artistProxyModel->setSourceModel(artistModel);
    ui->artistList->setModel(artistProxyModel);

    genresModel = new QStandardItemModel(this);
    genresProxyModel = new QSortFilterProxyModel(this);
    genresProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    genresProxyModel->setSourceModel(genresModel);
    ui->genresList->setModel(genresProxyModel);

    playlistModel = new QStandardItemModel(this);
    playlistProxyModel = new HeaderAwareProxyModel(this);
    playlistProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    playlistProxyModel->setSourceModel(playlistModel);
    ui->playlistList->setModel(playlistProxyModel);

    loadViewState();

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);

    browseRecentlyAddedId =
    browseRecentlyPlayedId =
    browseMostPlayedId =
    browseNeverPlayedId =
    browseImportedPlaylistsId = MAFW_SOURCE_INVALID_BROWSE_ID;
#endif

    connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

MusicWindow::~MusicWindow()
{
    delete ui;
}

void MusicWindow::onSongSelected(QModelIndex index)
{
    this->setEnabled(false);

#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(false);

    bool filter = QSettings().value("main/playlistFilter", false).toBool();

    int visibleCount = filter ? songProxyModel->rowCount() :
                                songModel->rowCount();

    gchar** songAddBuffer = new gchar*[visibleCount+1];

    if (filter)
        for (int i = 0; i < visibleCount; i++)
            songAddBuffer[i] = qstrdup(songProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());
    else
        for (int i = 0; i < visibleCount; i++)
            songAddBuffer[i] = qstrdup(songModel->item(i)->data(UserRoleObjectID).toString().toUtf8());

    songAddBuffer[visibleCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < visibleCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    mafwrenderer->gotoIndex(filter ? index.row() : songProxyModel->mapToSource(index).row());
    mafwrenderer->play();

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
#else
    NowPlayingWindow *window = NowPlayingWindow::acquire(this);
#endif

    window->show();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();
}

void MusicWindow::onPlaylistSelected(QModelIndex index)
{
    if (index.data(UserRoleHeader).toBool()) return;

    int row = playlistProxyModel->mapToSource(index).row();

    if (row >= 1 && row <= 4) {
        this->setEnabled(false);

        SinglePlaylistView *playlistView = new SinglePlaylistView(this, mafwFactory);
        playlistView->setWindowTitle(index.data(Qt::DisplayRole).toString());

        int limit = QSettings().value("music/playlistSize", 30).toInt();
        if (row == 1)
            playlistView->browseAutomaticPlaylist("", "-added", limit);
        else if (row == 2)
            playlistView->browseAutomaticPlaylist("(play-count>0)", "-last-played", limit);
        else if (row == 3)
            playlistView->browseAutomaticPlaylist("(play-count>0)", "-play-count,+title", limit);
        else if (row == 4)
            playlistView->browseAutomaticPlaylist("(play-count=)", "", MAFW_SOURCE_BROWSE_ALL);

        playlistView->show();
        connect(playlistView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

    } else if (row >= 6) {
        this->setEnabled(false);

        SinglePlaylistView *playlistView = new SinglePlaylistView(this, mafwFactory);
        playlistView->setWindowTitle(index.data(Qt::DisplayRole).toString());

        if (index.data(UserRoleObjectID).isNull())
            playlistView->browseSavedPlaylist(MAFW_PLAYLIST(mafwPlaylistManager->createPlaylist(index.data(Qt::DisplayRole).toString())));
        else
            playlistView->browseImportedPlaylist(index.data(UserRoleObjectID).toString());

        playlistView->show();
        connect(playlistView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();
    }
}

void MusicWindow::connectSignals()
{
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));
    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), ui->windowMenu), SIGNAL(activated()), ui->windowMenu, SLOT(close()));

#ifdef MAFW
    connect(ui->songList, SIGNAL(activated(QModelIndex)), this, SLOT(onSongSelected(QModelIndex)));
    connect(ui->albumList, SIGNAL(activated(QModelIndex)), this, SLOT(onAlbumSelected(QModelIndex)));
    connect(ui->artistList, SIGNAL(activated(QModelIndex)), this, SLOT(onArtistSelected(QModelIndex)));
    connect(ui->genresList, SIGNAL(activated(QModelIndex)), this, SLOT(onGenreSelected(QModelIndex)));
    connect(ui->playlistList, SIGNAL(activated(QModelIndex)), this, SLOT(onPlaylistSelected(QModelIndex)));

    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->albumList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->artistList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->genresList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->playlistList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));

    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseSourcePlaylists(uint,int,uint,QString,GHashTable*,QString)));
#endif
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->albumList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->artistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->genresList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->playlistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged()));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));
}

void MusicWindow::onContextMenuRequested(const QPoint &pos)
{
    if (currentList() == ui->playlistList && ui->playlistList->currentIndex().data(UserRoleHeader).toBool()) return;

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);

    // All views
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));

    // Artist, album or song view
    if (currentList() == ui->artistList || currentList() == ui->albumList || currentList() == ui->songList) {
        // Song view
        if (currentList() == ui->songList)
            contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
        // Everything
        contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
        // Song view
        if (currentList() == ui->songList) {
            contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
            contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
        }
    }

    // Playlist view
    else if (currentList() == ui->playlistList) {
        // Non-automatic playlist
        if (playlistProxyModel->mapToSource(ui->playlistList->currentIndex()).row() > 4) {
            // Saved playlist
            if (ui->playlistList->currentIndex().data(UserRoleObjectID).isNull()) {
                contextMenu->addAction(tr("Rename playlist"), this, SLOT(onRenamePlaylist()));
                contextMenu->addAction(tr("Delete playlist"), this, SLOT(onDeletePlaylistClicked()));
            // Imported playlist
            } else {
                contextMenu->addAction(tr("Delete playlist"), this, SLOT(onDeleteClicked()));
            }
        }
    }

    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), contextMenu), SIGNAL(activated()), contextMenu, SLOT(close()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void MusicWindow::showWindowMenu()
{
    ui->windowMenu->adjustSize();
    int x = (this->width() - ui->windowMenu->width()) / 2;
    ui->windowMenu->exec(this->mapToGlobal(QPoint(x,-35)));
    qDebug() << this->mapToGlobal(QPoint(x,-35));
}

void MusicWindow::onRenamePlaylist()
{
    renamePlaylistDialog = new QDialog(this);
    renamePlaylistDialog->setWindowTitle(tr("Rename playlist"));

    playlistNameEdit = new QLineEdit(ui->playlistList->currentIndex().data(Qt::DisplayRole).toString(), renamePlaylistDialog);
    playlistNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save, Qt::Horizontal, this);
    buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    buttonBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onRenamePlaylistAccepted()));

    QHBoxLayout *layout = new QHBoxLayout(renamePlaylistDialog);
    layout->addWidget(playlistNameEdit);
    layout->addWidget(buttonBox);

    renamePlaylistDialog->show();
}

void MusicWindow::onRenamePlaylistAccepted()
{
    QString oldName = ui->playlistList->currentIndex().data(Qt::DisplayRole).toString();
    QString newName = playlistNameEdit->text();

    if (newName == oldName) {
        renamePlaylistDialog->close();
    } else {
#ifdef MAFW
        GArray* playlists = mafwPlaylistManager->listPlaylists();
        for (uint i = 0; i < playlists->len; i++) {
            if (QString::fromUtf8(g_array_index(playlists, MafwPlaylistManagerItem, i).name) == newName) {
#ifdef Q_WS_MAEMO_5
                QMaemo5InformationBox::information(this, "Playlist with the same name exists");
#endif
                mafw_playlist_manager_free_list_of_playlists(playlists);
                return;
            }
        }
        mafw_playlist_manager_free_list_of_playlists(playlists);

        renamePlaylistDialog->close();
        mafw_playlist_set_name(MAFW_PLAYLIST(mafwPlaylistManager->createPlaylist(oldName)), newName.toUtf8());
        listSavedPlaylists();
    }
#endif
}

void MusicWindow::onDeletePlaylistClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwPlaylistManager->deletePlaylist(ui->playlistList->currentIndex().data(Qt::DisplayRole).toString());
        playlistProxyModel->removeRow(ui->playlistList->currentIndex().row());
        --savedPlaylistCount;
    }
#endif
    ui->playlistList->clearSelection();
}

void MusicWindow::setRingingTone()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Ringtone, this,
                      ui->songList->currentIndex().data(UserRoleSongArtist).toString(),
                      ui->songList->currentIndex().data(Qt::DisplayRole).toString())
        .exec() == QMessageBox::Yes)
    {
        mafwTrackerSource->getUri(ui->songList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
        connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
    }
#endif
    ui->songList->clearSelection();
}

#ifdef MAFW
void MusicWindow::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songList->currentIndex().data(UserRoleObjectID).toString())
        return;
#ifdef Q_WS_MAEMO_5
    QDBusInterface setRingtone("com.nokia.profiled",
                               "/com/nokia/profiled",
                               "com.nokia.profiled",
                               QDBusConnection::sessionBus(), this);
    setRingtone.call("set_value", "general", "ringing.alert.tone", uri);
    QMaemo5InformationBox::information(this, tr("Selected song set as ringing tone"));
#endif
}
#endif

void MusicWindow::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void MusicWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentIndex().data(UserRoleObjectID).toString())
        return;

    QStringList files;
    files.append(uri);
#ifdef Q_WS_MAEMO_5
    ShareDialog(this, files).exec();
#endif
}
#endif

void MusicWindow::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(currentList()->currentIndex().data(UserRoleObjectID).toString().toUtf8());
        currentList()->model()->removeRow(currentList()->currentIndex().row());
    }
#endif
    currentList()->clearSelection();
}


void MusicWindow::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void MusicWindow::onSearchTextChanged()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void MusicWindow::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
}

bool MusicWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->albumList->viewport() && e->type() == QEvent::Resize)
        ui->albumList->setFlow(ui->albumList->flow());

    else if (obj == ui->playlistList->viewport() && e->type() == QEvent::WindowActivate)
        listSavedPlaylists();

    else if (e->type() == QEvent::MouseButtonPress
         && ((QMouseEvent*)e)->y() > currentList()->viewport()->height() - 25
         && ui->searchWidget->isHidden()) {
             ui->indicator->inhibit();
             ui->searchWidget->show();
         }
    return false;
}

void MusicWindow::populateWindowMenu()
{
    ui->windowMenu->clear();

    if (ui->albumList->isHidden())
        ui->windowMenu->addAction(tr("All albums"), this, SLOT(showAlbumView()));
    if (ui->artistList->isHidden())
        ui->windowMenu->addAction(tr("Artists"), this, SLOT(showArtistView()));
    if (ui->songList->isHidden())
        ui->windowMenu->addAction(tr("All songs"), this, SLOT(showSongsView()));
    if (ui->genresList->isHidden())
        ui->windowMenu->addAction(tr("Genres"), this, SLOT(showGenresView()));
    if (ui->playlistList->isHidden())
        ui->windowMenu->addAction(tr("Playlists"), this, SLOT(showPlayListView()));
}

void MusicWindow::hideLayoutContents()
{
    // Prevent focus from disapearing when hiding lists to keep shortcuts working
    this->setFocus();

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

void MusicWindow::disconnectSearch()
{
    disconnect(ui->searchEdit, SIGNAL(textChanged(QString)), songProxyModel, SLOT(setFilterFixedString(QString)));
    disconnect(ui->searchEdit, SIGNAL(textChanged(QString)), albumProxyModel, SLOT(setFilterFixedString(QString)));
    disconnect(ui->searchEdit, SIGNAL(textChanged(QString)), artistProxyModel, SLOT(setFilterFixedString(QString)));
    disconnect(ui->searchEdit, SIGNAL(textChanged(QString)), genresProxyModel, SLOT(setFilterFixedString(QString)));
    disconnect(ui->searchEdit, SIGNAL(textChanged(QString)), playlistProxyModel, SLOT(setFilterFixedString(QString)));
}

void MusicWindow::showAlbumView()
{
    ui->searchEdit->clear();
    disconnectSearch();
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), albumProxyModel, SLOT(setFilterFixedString(QString)));

    this->hideLayoutContents();
    ui->albumList->show();
    this->populateWindowMenu();
    QMainWindow::setWindowTitle(tr("Albums"));
    this->saveViewState("albums");
#ifdef MAFW
    if (albumModel->rowCount() == 0) {
        if(mafwTrackerSource->isReady())
            this->listAlbums();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listAlbums()));
    }
#endif
}

void MusicWindow::showArtistView()
{
    ui->searchEdit->clear();
    disconnectSearch();
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), artistProxyModel, SLOT(setFilterFixedString(QString)));

    this->hideLayoutContents();
    ui->artistList->show();
    this->populateWindowMenu();
    QMainWindow::setWindowTitle(tr("Artists"));
    this->saveViewState("artists");
#ifdef MAFW
    if (artistModel->rowCount() == 0) {
        if(mafwTrackerSource->isReady())
            this->listArtists();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listArtists()));
    }
#endif
}

void MusicWindow::showGenresView()
{
    ui->searchEdit->clear();
    disconnectSearch();
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), genresProxyModel, SLOT(setFilterFixedString(QString)));

    this->hideLayoutContents();
    ui->genresList->show();
    this->populateWindowMenu();
    QMainWindow::setWindowTitle(tr("Genres"));
    this->saveViewState("genres");
#ifdef MAFW
    if (genresModel->rowCount() == 0) {
        if (mafwTrackerSource->isReady())
            this->listGenres();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listGenres()));
    }
#endif
}

void MusicWindow::showSongsView()
{
    ui->searchEdit->clear();
    disconnectSearch();
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), songProxyModel, SLOT(setFilterFixedString(QString)));

    this->hideLayoutContents();
    ui->songList->show();
    this->populateWindowMenu();
    QMainWindow::setWindowTitle(tr("Songs"));
    this->saveViewState("songs");
#ifdef MAFW
    if (songModel->rowCount() == 0) {
        if(mafwTrackerSource->isReady())
            this->listSongs();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
    }
#endif
}

void MusicWindow::showPlayListView()
{
    ui->searchEdit->clear();
    disconnectSearch();
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), playlistProxyModel, SLOT(setFilterFixedString(QString)));

    this->hideLayoutContents();
    ui->playlistList->show();
    this->populateWindowMenu();
    QMainWindow::setWindowTitle(tr("Playlists"));
    this->saveViewState("playlists");
#ifdef MAFW
    if (playlistModel->rowCount() == 0) {
        savedPlaylistCount = 0;
        if (mafwTrackerSource->isReady())
            this->listPlaylists();
        else
            connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listPlaylists()), Qt::UniqueConnection);
    }
#endif
}

void MusicWindow::refreshPlaylistView()
{
    if (playlistModel->rowCount() != 0) listPlaylists();
}

QListView* MusicWindow::currentList()
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

void MusicWindow::saveViewState(QString view)
{
    QSettings().setValue("music/view", view);
}

void MusicWindow::loadViewState()
{
    QString state = QSettings().value("music/view", "songs").toString();

    if (state == "albums")
        this->showAlbumView();
    else if (state == "artists")
        this->showArtistView();
    else if (state == "genres")
        this->showGenresView();
    else if (state == "playlists")
        this->showPlayListView();
    else // state == "songs"
        this->showSongsView();
}

void MusicWindow::onChildClosed()
{
    ui->indicator->restore();
    this->currentList()->clearSelection();
    this->setEnabled(true);
}

#ifdef MAFW
void MusicWindow::onAlbumSelected(QModelIndex index)
{
    this->setEnabled(false);

    SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
    albumView->browseAlbumByObjectId(index.data(UserRoleObjectID).toString());
    albumView->setWindowTitle(index.data(UserRoleTitle).toString());

    albumView->show();

    connect(albumView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}

void MusicWindow::onArtistSelected(QModelIndex index)
{
    int songCount = index.data(UserRoleAlbumCount).toInt();

    if (songCount == 0 || songCount == 1) {
        this->setEnabled(false);

        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        albumView->browseAlbumByObjectId(index.data(UserRoleObjectID).toString());
        albumView->setWindowTitle(index.data(Qt::DisplayRole).toString());

        albumView->show();
        connect(albumView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

    } else if (songCount > 1) {
        this->setEnabled(false);

        SingleArtistView *artistView = new SingleArtistView(this, mafwFactory);
        artistView->browseAlbum(index.data(UserRoleObjectID).toString());
        artistView->setWindowTitle(index.data(Qt::DisplayRole).toString());

        artistView->show();
        connect(artistView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();
    }
}

void MusicWindow::onGenreSelected(QModelIndex index)
{
    this->setEnabled(false);

    SingleGenreView *genreView = new SingleGenreView(this, mafwFactory);
    genreView->setWindowTitle(index.data(Qt::DisplayRole).toString());

    genreView->show();

    connect(genreView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();

    genreView->browseGenre(index.data(UserRoleObjectID).toString());
}

void MusicWindow::listSongs()
{
    qDebug() << "Updating songs";

#ifdef DEBUG
    qDebug() << "MusicWindow: Source ready";
#endif
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    songModel->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllSongs(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseAllSongsId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, NULL,
                                                       MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                        MAFW_METADATA_KEY_ALBUM,
                                                                        MAFW_METADATA_KEY_ARTIST,
                                                                        MAFW_METADATA_KEY_DURATION),
                                                       0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listArtists()
{
    qDebug() << "Updating artists";

#ifdef DEBUG
    qDebug("Source ready");
#endif
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    artistModel->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllArtists(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseAllArtistsId = mafwTrackerSource->sourceBrowse("localtagfs::music/artists", false, NULL, NULL,
                                                         MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                          MAFW_METADATA_KEY_DURATION,
                                                                          MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                          MAFW_METADATA_KEY_CHILDCOUNT_2,
                                                                          MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI),
                                                         0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listAlbums()
{
    qDebug() << "Updating albums";

#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    albumModel->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllAlbums(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseAllAlbumsId = mafwTrackerSource->sourceBrowse("localtagfs::music/albums", false, NULL, NULL,
                                                        MAFW_SOURCE_LIST(MAFW_METADATA_KEY_ALBUM,
                                                                         MAFW_METADATA_KEY_ARTIST,
                                                                         MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                         MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI),
                                                        0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listGenres()
{
    qDebug() << "Updating genres";

#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    genresModel->clear();
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllGenres(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseAllGenresId = mafwTrackerSource->sourceBrowse("localtagfs::music/genres", false, NULL, NULL,
                                                        MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                         MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                         MAFW_METADATA_KEY_CHILDCOUNT_2,
                                                                         MAFW_METADATA_KEY_CHILDCOUNT_3),
                                                        0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::listPlaylists()
{
    playlistModel->clear();
    savedPlaylistCount = 0;

    this->listAutoPlaylists();
    this->listSavedPlaylists();
    this->listImportedPlaylists();
}

void MusicWindow::listAutoPlaylists()
{
    qDebug() << "Updating automatic playlists";

    QStandardItem *item = new QStandardItem();
    item->setText(tr("Automatic playlists"));
    item->setData(true, UserRoleHeader);
    playlistModel->appendRow(item);

    int limit = QSettings().value("music/playlistSize", 30).toInt();
    QStringList playlists;
    playlists << tr("Recently added") << tr("Recently played") << tr("Most played") << tr("Never played");
    foreach (QString string, playlists) {
        QStandardItem *item = new QStandardItem();
        item->setText(string);
        item->setData(Duration::Blank, UserRoleSongDuration);
        playlistModel->appendRow(item);
    }

    browseNeverPlayedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false,
                                                          "(play-count=)",
                                                          NULL,
                                                          MAFW_SOURCE_LIST(),
                                                          0, MAFW_SOURCE_BROWSE_ALL);

    browseMostPlayedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false,
                                                         "(play-count>0)",
                                                         "-play-count,+title",
                                                         MAFW_SOURCE_LIST(),
                                                         0, limit);

    browseRecentlyPlayedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false,
                                                             "(play-count>0)",
                                                             "-last-played",
                                                             MAFW_SOURCE_LIST(),
                                                             0, limit);

    browseRecentlyAddedId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", false, NULL, "-added",
                                                            MAFW_SOURCE_LIST(),
                                                            0, limit);
}

void MusicWindow::listSavedPlaylists()
{
    qDebug() << "Updating saved playlists";

    playlistModel->removeRows(5, savedPlaylistCount);

    GArray* playlists = mafwPlaylistManager->listPlaylists();
    savedPlaylistCount = 0;

    if (playlists->len != 0) {
        QStandardItem *item = new QStandardItem();
        item->setText(tr("Saved"));
        item->setData(true, UserRoleHeader);
        playlistModel->insertRow(5, item);
        ++savedPlaylistCount;

        for (int i = playlists->len-1; i >= 0; i--) {
            MafwPlaylistManagerItem* plItem = &g_array_index(playlists, MafwPlaylistManagerItem, i);

            QString playlistName = QString::fromUtf8(plItem->name);
            int playlistSize = playlist->getSizeOf(MAFW_PLAYLIST (mafwPlaylistManager->getPlaylist(plItem->id)));

            if (playlistName != "FmpAudioPlaylist"
            && playlistName != "FmpVideoPlaylist"
            && playlistName != "FmpRadioPlaylist") {
                item = new QStandardItem();
                item->setText(playlistName);
                item->setData(playlistSize, UserRoleSongCount);
                item->setData(Duration::Blank, UserRoleSongDuration);
                item->setData(tr("%n song(s)", "", playlistSize),UserRoleValueText);

                playlistModel->insertRow(6, item);
                ++savedPlaylistCount;
            }
        }
    }

    mafw_playlist_manager_free_list_of_playlists(playlists);
}

void MusicWindow::listImportedPlaylists()
{
    qDebug() << "Updating imported playlists";

    browseImportedPlaylistsId = mafwTrackerSource->sourceBrowse("localtagfs::music/playlists", false, NULL, NULL,
                                                                MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                                 MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                                 MAFW_METADATA_KEY_DURATION),
                                                                0, MAFW_SOURCE_BROWSE_ALL);
}

void MusicWindow::browseSourcePlaylists(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString error)
{
    if (!error.isEmpty()) {
        qDebug() << error;
        error = QString();
    }

    GValue *v;

    if (browseId == this->browseRecentlyAddedId) {
        mafwTrackerSource->cancelBrowse(browseId, error);
        browseRecentlyAddedId = MAFW_SOURCE_INVALID_BROWSE_ID;
        int size = remainingCount == 0 && objectId.isNull() ? 0 : remainingCount+1;
        playlistModel->item(1)->setData(tr("%n song(s)", "", size), UserRoleValueText);

    } else if (browseId == this->browseRecentlyPlayedId) {
        mafwTrackerSource->cancelBrowse(browseId, error);
        browseRecentlyPlayedId = MAFW_SOURCE_INVALID_BROWSE_ID;
        int size = remainingCount == 0 && objectId.isNull() ? 0 : remainingCount+1;
        playlistModel->item(2)->setData(tr("%n song(s)", "", size), UserRoleValueText);

    } else if (browseId == this->browseMostPlayedId) {
        mafwTrackerSource->cancelBrowse(browseId, error);
        browseMostPlayedId = MAFW_SOURCE_INVALID_BROWSE_ID;
        int size = remainingCount == 0 && objectId.isNull() ? 0 : remainingCount+1;
        playlistModel->item(3)->setData(tr("%n song(s)", "", size), UserRoleValueText);

    } else if (browseId == this->browseNeverPlayedId) {
        mafwTrackerSource->cancelBrowse(browseId, error);
        browseNeverPlayedId = MAFW_SOURCE_INVALID_BROWSE_ID;
        int size = remainingCount == 0 && objectId.isNull() ? 0 : remainingCount+1;
        playlistModel->item(4)->setData(tr("%n song(s)", "", size), UserRoleValueText);

    } else if (browseId == this->browseImportedPlaylistsId) {
        if (index == 0) {
            if (remainingCount == 0 && objectId.isNull()) return;

            QStandardItem *item = new QStandardItem();
            item->setText(tr("Imported playlists"));
            item->setData(true, UserRoleHeader);
            playlistModel->appendRow(item);
        }

        QStandardItem *item = new QStandardItem();

        v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_TITLE);
        item->setText(QString::fromUtf8(g_value_get_string(v)));

        v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
        item->setData(tr("%n song(s)", "", g_value_get_int(v)), UserRoleValueText);

        item->setData(Duration::Blank, UserRoleSongDuration);
        item->setData(objectId, UserRoleObjectID);

        playlistModel->appendRow(item);
    }

    if (!error.isEmpty())
        qDebug() << error;
}

void MusicWindow::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != browseAllSongsId) return;

    if (metadata != NULL) {
        QString title;
        QString artist;
        QString album;
        int duration;
        GValue *v;

        QStandardItem *item = new QStandardItem();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        item->setText(title);
        item->setData(artist, UserRoleSongArtist);
        item->setData(album, UserRoleSongAlbum);
        item->setData(objectId, UserRoleObjectID);
        item->setData(duration, UserRoleSongDuration);

        item->setData(QString(title % QChar(31) % artist % QChar(31) % album), UserRoleFilterString);
        songModel->appendRow(item);
    }

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllSongs(uint,int,uint,QString,GHashTable*,QString)));
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void MusicWindow::browseAllArtists(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != browseAllArtistsId) return;

    if (metadata != NULL) {
        QString title;
        int songCount;
        int albumCount;
        GValue *v;

        QStandardItem *item = new QStandardItem();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
        albumCount = v ? g_value_get_int (v) : -1;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_2);
        songCount = v ? g_value_get_int (v) : -1;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(filename, UserRoleAlbumArt);
            }
        }

        if (title.isEmpty()) title = tr("(unknown artist)");

        item->setText(title);
        item->setData(songCount, UserRoleSongCount);
        item->setData(albumCount, UserRoleAlbumCount);
        item->setData(objectId, UserRoleObjectID);

        artistModel->appendRow(item);
    }

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllArtists(uint,int,uint,QString,GHashTable*,QString)));
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void MusicWindow::browseAllAlbums(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != browseAllAlbumsId) return;

    if (metadata != NULL) {
        QString albumTitle;
        QString artist;
        QString albumArt;
        int songCount;
        GValue *v;

        QStandardItem *item = new QStandardItem();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
        songCount = v ? g_value_get_int(v) : Duration::Unknown;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                item->setIcon(QIcon(QString::fromUtf8(filename)));
        } else {
            item->setIcon(QIcon::fromTheme(defaultAlbumIcon));
        }

        if (artist == "__VV__") artist = tr("Various artists");

        item->setData(albumTitle, UserRoleTitle);
        item->setData(artist, UserRoleValueText);
        item->setData(objectId, UserRoleObjectID);
        item->setData(songCount, UserRoleSongCount);

        item->setData(QString(albumTitle % QChar(31) % artist), UserRoleFilterString);
        albumModel->appendRow(item);
    }

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllAlbums(uint,int,uint,QString,GHashTable*,QString)));

#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void MusicWindow::browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable *metadata, QString error)
{
    if (this->browseAllGenresId != browseId) return;

    QString title;
    int songCount;
    int albumCount;
    int artistCount;
    GValue *v;

    QStandardItem *item = new QStandardItem();

    v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_TITLE);
    title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown genre)");

    v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
    artistCount = v ? g_value_get_int (v) : -1;

    v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_CHILDCOUNT_2);
    albumCount = v ? g_value_get_int (v) : -1;

    v = mafw_metadata_first (metadata, MAFW_METADATA_KEY_CHILDCOUNT_3);
    songCount = v ? g_value_get_int (v) : -1;

    if (title.isEmpty()) title = tr("(unknown genre)");

    item->setText(title);
    item->setData(songCount, UserRoleSongCount);
    item->setData(artistCount, UserRoleArtistCount);
    item->setData(albumCount, UserRoleAlbumCount);
    item->setData(objectId, UserRoleObjectID);
    item->setData(Duration::Blank, UserRoleSongDuration);

    QString valueText = tr("%n song(s)", "", songCount) + ", "
                      + tr("%n album(s)", "", albumCount) + ", "
                      + tr("%n artist(s)", "", artistCount);
    item->setData(valueText, UserRoleValueText);

    genresModel->appendRow(item);

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllGenres(uint,int,uint,QString,GHashTable*,QString)));
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}
#endif

void MusicWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void MusicWindow::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Backspace:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            return;

        case Qt::Key_Up:
        case Qt::Key_Down:
            currentList()->setFocus();
            break;

        default:
            currentList()->clearSelection();
            if (ui->searchWidget->isHidden()) {
                ui->indicator->inhibit();
                ui->searchWidget->show();
            }
            if (!ui->searchEdit->hasFocus()) {
                ui->searchEdit->setText(ui->searchEdit->text() + e->text());
                ui->searchEdit->setFocus();
            }
            break;
    }
}

void MusicWindow::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    // Song list, add the selected song
    if (currentList() == ui->songList) {
        playlist->appendItem(ui->songList->currentIndex().data(UserRoleObjectID).toString());
#ifdef Q_WS_MAEMO_5
        notifyOnAddedToNowPlaying(1);
#endif
    }

    // Artist/album/genre list, add items from the selected artist/album/genre
    else if (currentList() == ui->artistList || currentList() == ui->albumList || currentList() == ui->genresList) {
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

        QString objectIdToBrowse = currentList()->currentIndex().data(UserRoleObjectID).toString();

        songAddBufferSize = 0;

        connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                this, SLOT(onAddToNowPlayingCallback(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

        addToNowPlayingId = mafwTrackerSource->sourceBrowse(objectIdToBrowse.toUtf8(), true, NULL, NULL, 0,
                                                            0, MAFW_SOURCE_BROWSE_ALL);
    }

    // Playlist list, add items from the selected playlist
    else if (currentList() == ui->playlistList) {
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
        QModelIndex index = ui->playlistList->currentIndex();
        int row = playlistProxyModel->mapToSource(index).row();

        // Automatic playlist
        if (row < 5) {
            QString filter;
            QString sorting;
            int limit = QSettings().value("music/playlistSize", 30).toInt();
            switch (row) {
                case 1: filter = ""; sorting = "-added"; break;
                case 2: filter = "(play-count>0)"; sorting = "-last-played"; break;
                case 3: filter = "(play-count>0)"; sorting = "-play-count,+title"; break;
                case 4: filter = "(play-count=)"; sorting = ""; limit = MAFW_SOURCE_BROWSE_ALL; break;
            }

            songAddBufferSize = 0;

            connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                    this, SLOT(onAddToNowPlayingCallback(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

            addToNowPlayingId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", true, filter.toUtf8(), sorting.toUtf8(),
                                                                MAFW_SOURCE_NO_KEYS, 0, limit);
        }

        // Saved playlist
        else if (index.data(UserRoleObjectID).isNull()) {

            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
            QApplication::processEvents();

            MafwPlaylist *mafwplaylist = MAFW_PLAYLIST(mafwPlaylistManager->createPlaylist(index.data(Qt::DisplayRole).toString()));
            int size = playlist->getSizeOf(mafwplaylist);
            gchar** items = mafw_playlist_get_items(mafwplaylist, 0, size-1, NULL);
            playlist->appendItems((const gchar**)items);
            g_strfreev(items);

            setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
            this->notifyOnAddedToNowPlaying(size);
        }

        // Imported playlist
        else {
            QString objectId = index.data(UserRoleObjectID).toString();

            songAddBufferSize = 0;

            connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                    this, SLOT(onAddToNowPlayingCallback(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

            // file size is not really needed, but MAFW_SOURCE_NO_KEYS would result in urisource IDs
            addToNowPlayingId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                                MAFW_SOURCE_LIST (MAFW_METADATA_KEY_FILESIZE),
                                                                0, MAFW_SOURCE_BROWSE_ALL);
        }
    }
#endif
}

void MusicWindow::onAddToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
#ifdef MAFW
        playlist->appendItem(picker.playlist, ui->songList->currentIndex().data(UserRoleObjectID).toString());
#endif
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
#endif
    }
}

void MusicWindow::onContainerChanged(QString objectId)
{
    qDebug() << "Container changed:" << objectId;

    if (objectId.startsWith("localtagfs::music")) {
        if (!objectId.endsWith("/playlists")) {
            listArtists();
            listGenres();
            listAlbums();
            listSongs();
        }
        listPlaylists();
    }
}

void MusicWindow::onAddToNowPlayingCallback(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString error)
{
    if (browseId != this->addToNowPlayingId) return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        playlist->appendItems((const gchar**)songAddBuffer);

        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onAddToNowPlayingCallback(uint,int,uint,QString,GHashTable*,QString)));

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
        notifyOnAddedToNowPlaying(songAddBufferSize);
#endif
    }

    if (!error.isEmpty())
        qDebug() << error;
}

#ifdef Q_WS_MAEMO_5
void MusicWindow::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void MusicWindow::showEvent(QShowEvent *)
{
    emit shown();
}

void MusicWindow::hideEvent(QHideEvent *)
{
    QMainWindow *child = findChild<QMainWindow*>();
    if (child) child->close();

    emit hidden();
}

void MusicWindow::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    this->onChildClosed();
}
