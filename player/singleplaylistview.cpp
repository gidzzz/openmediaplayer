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

#include "singleplaylistview.h"

SinglePlaylistView::SinglePlaylistView(QWidget *parent, MafwAdapterFactory *factory) :
    BaseWindow(parent),
    ui(new Ui::SinglePlaylistView)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

#ifdef MAFW
    ui->indicator->setFactory(factory);
    browsePlaylistId = MAFW_SOURCE_INVALID_BROWSE_ID;
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif

    permanentDelete = QSettings().value("main/permanentDelete").toBool();

    SongListItemDelegate *songDelegate = new SongListItemDelegate(ui->songList);
    ShuffleButtonDelegate *shuffleDelegate = new ShuffleButtonDelegate(ui->songList);
    ui->songList->setItemDelegate(songDelegate);
    ui->songList->setItemDelegateForRow(0, shuffleDelegate);

    songModel = new QStandardItemModel(this);
    songProxyModel = new HeaderAwareProxyModel(this);
    songProxyModel->setFilterRole(UserRoleFilterString);
    songProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    songProxyModel->setSourceModel(songModel);
    ui->songList->setModel(songProxyModel);

    ui->songList->viewport()->installEventFilter(this);

    ui->songList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->songList->viewport()->setAcceptDrops(true);
    ui->songList->setAutoScrollMargin(70);
    QApplication::setStartDragDistance(20);
    ui->songList->setDragEnabled(false);

    playlistModified = false;
    pendingActivation = Nothing;

    clickedIndex = QModelIndex();
    clickTimer = new QTimer(this);
    clickTimer->setInterval(QApplication::doubleClickInterval());
    clickTimer->setSingleShot(true);

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));

    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
    connect(ui->actionAdd_to_playlist, SIGNAL(triggered()), this, SLOT(addAllToPlaylist()));
    connect(ui->actionDelete_playlist, SIGNAL(triggered()), this, SLOT(deletePlaylist()));

    connect(clickTimer, SIGNAL(timeout()), this, SLOT(forgetClick()));

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

SinglePlaylistView::~SinglePlaylistView()
{
    delete ui;
}

void SinglePlaylistView::browseSavedPlaylist(MafwPlaylist *mafwplaylist)
{
    currentObjectId = QString();
    playlistLoaded = true;

    songModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    item->setDragEnabled(false);
    item->setDropEnabled(false);
    songModel->appendRow(item);

    connect(ui->songList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked()));

    int size = playlist->getSizeOf(mafwplaylist);

    gchar** items = mafw_playlist_get_items(mafwplaylist, 0, size-1, NULL);
    for (int i = 0; items[i] != NULL; i++) {
        QStandardItem *item = new QStandardItem();
        item->setData(QString::fromUtf8(items[i]), UserRoleObjectID);
        item->setData(Duration::Blank, UserRoleSongDuration);
        item->setDropEnabled(false);
        songModel->appendRow(item);
    }
    g_strfreev(items);

    playlistQM = new PlaylistQueryManager(this, playlist, mafwplaylist);
    connect(playlistQM, SIGNAL(onGetItems(QString, GHashTable*, guint)), this, SLOT(onGetItems(QString, GHashTable*, guint)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), playlistQM, SLOT(setPriority(int)));
    playlistQM->getItems(0, size-1);

    remainingCount = size;
    updateSongCount();
}

void SinglePlaylistView::onGetItems(QString objectId, GHashTable* metadata, guint index)
{
    remainingCount--;

    if (playlistModified) {
        for (int i = 1; i < songModel->rowCount(); i++)
            if (songModel->item(i)->data(UserRoleObjectID).toString() == objectId)
                setItemMetadata(songModel->item(i), objectId, metadata);
    } else {
        setItemMetadata(songModel->item(index+1), objectId, metadata);
    }

#ifdef Q_WS_MAEMO_5
    if (remainingCount == 0)
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}

void SinglePlaylistView::setItemMetadata(QStandardItem *item, QString objectId, GHashTable *metadata)
{
    if (metadata != NULL) {
        QString title;
        QString artist;
        QString album;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        item->setText(title);
        item->setData(duration, UserRoleSongDuration);
        item->setData(album, UserRoleSongAlbum);
        item->setData(artist, UserRoleSongArtist);
        item->setData(objectId, UserRoleObjectID);
        item->setData(QString(title % QChar(31) % artist % QChar(31) % album), UserRoleFilterString);

    } else {
        item->setText(tr("Information not available"));
        item->setData(Duration::Blank, UserRoleSongDuration);
    }
}

void SinglePlaylistView::browseImportedPlaylist(QString objectId)
{
    currentObjectId = objectId;
    playlistLoaded = false;

    songModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    songModel->appendRow(item);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);
    browsePlaylistId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                       MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                        MAFW_METADATA_KEY_DURATION,
                                                                        MAFW_METADATA_KEY_ARTIST,
                                                                        MAFW_METADATA_KEY_ALBUM),
                                                       0, MAFW_SOURCE_BROWSE_ALL);
}

void SinglePlaylistView::browseAutomaticPlaylist(QString filter, QString sorting, int maxCount)
{
    currentObjectId = QString();
    playlistLoaded = false;

    songModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    songModel->appendRow(item);

    ui->windowMenu->removeAction(ui->actionDelete_playlist);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);
    browsePlaylistId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", true, filter.toUtf8(), sorting.toUtf8(),
                                                       MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                        MAFW_METADATA_KEY_DURATION,
                                                                        MAFW_METADATA_KEY_ARTIST,
                                                                        MAFW_METADATA_KEY_ALBUM),
                                                       0, maxCount);
}

void SinglePlaylistView::onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString)
{
    if (browseId != this->browsePlaylistId) return;

    if (index != 0 || remainingCount != 0 || !objectId.isNull()) {
        QStandardItem *item = new QStandardItem();
        setItemMetadata(item, objectId, metadata);
        songModel->appendRow(item);
        updateSongCount();
    }

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
        playlistLoaded = true;

        switch (pendingActivation) {
            case Nothing:
                break;

            case AddToNowPlaying:
                addAllToNowPlaying();
                break;

            case AddToPlaylist:
                addAllToPlaylist();
                break;

            default:
                onItemActivated(songProxyModel->mapFromSource(songModel->index(pendingActivation,0)));
                break;
        }
    }
}

void SinglePlaylistView::onItemActivated(QModelIndex index)
{
#ifdef MAFW
    this->setEnabled(false);

    if (!playlistLoaded) {
        pendingActivation = songProxyModel->mapToSource(index).row();
        return;
    }

    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(index.row() == 0);

    bool filter = index.row() == 0 || QSettings().value("main/playlistFilter", false).toBool();

    appendAllToPlaylist(filter);

    mafwrenderer->gotoIndex((filter ? index.row() : songProxyModel->mapToSource(index).row())-1);
    mafwrenderer->play();

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
    window->show();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();
#endif
}

void SinglePlaylistView::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
}

void SinglePlaylistView::addAllToNowPlaying()
{
    if (!playlistLoaded) {
        pendingActivation = AddToNowPlaying;
        return;
    }

#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

#ifdef Q_WS_MAEMO_5
    notifyOnAddedToNowPlaying(appendAllToPlaylist(true));
#endif

#endif
}

void SinglePlaylistView::addAllToPlaylist()
{
    if (!playlistLoaded) {
        pendingActivation = AddToPlaylist;
        return;
    }

    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
        int songCount = songProxyModel->rowCount();

        if (currentObjectId.isNull() && picker.playlistName == windowTitle()) {
            for (int i = 1; i < songCount; i++)
                songModel->appendRow(songModel->item(songProxyModel->mapToSource(songProxyModel->index(i,0)).row())->clone());
            updateSongCount();
            playlistModified = true;
            --songCount;
        } else {
#ifdef MAFW
            gchar** songAddBuffer = new gchar*[songCount];

            for (int i = 1; i < songCount; i++)
                songAddBuffer[i-1] = qstrdup(songProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());

            songAddBuffer[--songCount] = NULL;

            playlist->appendItems(picker.playlist, (const gchar**) songAddBuffer);

            for (int i = 0; i < songCount; i++)
                delete[] songAddBuffer[i];
            delete[] songAddBuffer;
#endif
        }

#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", songCount));
#endif
    }
}

int SinglePlaylistView::appendAllToPlaylist(bool filter)
{
#ifdef MAFW
    int visibleCount = filter ? songProxyModel->rowCount() : songModel->rowCount();

    gchar** songAddBuffer = new gchar*[visibleCount];

    if (filter)
        for (int i = 1; i < visibleCount; i++)
            songAddBuffer[i-1] = qstrdup(songProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());
    else
        for (int i = 1; i < visibleCount; i++)
            songAddBuffer[i-1] = qstrdup(songModel->item(i)->data(UserRoleObjectID).toString().toUtf8());

    songAddBuffer[--visibleCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < visibleCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    return visibleCount;
#endif
}

#ifdef Q_WS_MAEMO_5
void SinglePlaylistView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void SinglePlaylistView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            break;

        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_Enter:
            onItemActivated(ui->songList->currentIndex());
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->songList->setFocus();
            break;

        default:
            ui->songList->clearSelection();
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

void SinglePlaylistView::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->songList->setFocus();
    }
}

void SinglePlaylistView::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void SinglePlaylistView::onSearchTextChanged(QString text)
{
    songProxyModel->setFilterFixedString(text);
    updateSongCount();

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SinglePlaylistView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->songList->currentIndex().row() <= 0) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    if (currentObjectId.isNull()) contextMenu->addAction(tr("Delete from playlist"), this, SLOT(onDeleteFromPlaylist()));
    if (permanentDelete) contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void SinglePlaylistView::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->songList->currentIndex().data(UserRoleObjectID).toString());

#ifdef Q_WS_MAEMO_5
    notifyOnAddedToNowPlaying(1);
#endif

#endif
}

void SinglePlaylistView::onAddToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
        if (currentObjectId.isNull() && picker.playlistName == windowTitle()) {
            songModel->appendRow(songModel->item(songProxyModel->mapToSource(ui->songList->currentIndex()).row())->clone());
            updateSongCount();
            playlistModified = true;
        }
#ifdef MAFW
        else
            playlist->appendItem(picker.playlist, ui->songList->currentIndex().data(UserRoleObjectID).toString());
#endif
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
#endif
    }
}

void SinglePlaylistView::setRingingTone()
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
void SinglePlaylistView::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songList->currentIndex().data(UserRoleObjectID).toString()) return;

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

void SinglePlaylistView::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void SinglePlaylistView::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentIndex().data(UserRoleObjectID).toString()) return;

    QStringList files;
#ifdef DEBUG
    qDebug() << "Sending file:" << uri;
#endif
    files.append(uri);
#ifdef Q_WS_MAEMO_5
    ShareDialog(files, this).exec();
#endif
}
#endif

void SinglePlaylistView::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->songList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
        songProxyModel->removeRow(ui->songList->currentIndex().row());
        updateSongCount();
        playlistModified = true;
    }
#endif
    ui->songList->clearSelection();
}

void SinglePlaylistView::updateSongCount()
{
    songModel->item(0)->setData(songProxyModel->rowCount()-1, UserRoleSongCount);
}

void SinglePlaylistView::forgetClick()
{
    if (clickedIndex.row() != -1) onItemActivated(clickedIndex);
    ui->songList->setDragEnabled(false);
    clickedIndex = QModelIndex();
}

bool SinglePlaylistView::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent*>(e);
        if (ui->songList->indexAt(de->pos()).row() == 0) {
            de->setDropAction(Qt::IgnoreAction);
        } else {
            de->setDropAction(Qt::MoveAction);
            playlistModified = true;
        }
    }

    else if (e->type() == QEvent::MouseButtonPress) {
        if (static_cast<QMouseEvent*>(e)->y() > ui->songList->viewport()->height() - 25
        && ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
        clickedIndex = ui->songList->indexAt(QPoint(0,static_cast<QMouseEvent*>(e)->y()));
    }

    else if (e->type() == QEvent::MouseButtonRelease) {
        if (clickedIndex != ui->songList->currentIndex())
            clickedIndex = QModelIndex();
        clickTimer->start();
    }

    return false;
}

void SinglePlaylistView::onItemDoubleClicked()
{
    if (ui->songList->currentIndex().row() != 0) {
        ui->songList->setDragEnabled(true);
        clickedIndex = QModelIndex();
        clickTimer->start();
    }
}

void SinglePlaylistView::saveCurrentPlaylist()
{
#ifdef MAFW
    MafwPlaylist *targetPlaylist = MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist(this->windowTitle()));
    playlist->clear(targetPlaylist);

    int songCount = songModel->rowCount();
    gchar** songAddBuffer = new gchar*[songCount];

    for (int i = 1; i < songCount; i++)
        songAddBuffer[i-1] = qstrdup(songModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[--songCount] = NULL;

    playlist->appendItems(targetPlaylist, (const gchar**) songAddBuffer);

    for (int i = 0; i < songCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    playlistModified = false;
#endif
}

void SinglePlaylistView::deletePlaylist()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::DeletePlaylist, this).exec() == QMessageBox::Yes) {
        if (currentObjectId.isNull()) // Saved playlist
            MafwPlaylistManagerAdapter::get()->deletePlaylist(this->windowTitle());
        else // Imported playlist
            mafwTrackerSource->destroyObject(currentObjectId.toUtf8());
        this->close();
    }
#endif
}

void SinglePlaylistView::onDeleteFromPlaylist()
{
    songProxyModel->removeRow(ui->songList->currentIndex().row());
    updateSongCount();
    playlistModified = true;
}

void SinglePlaylistView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->restore();
    ui->songList->clearSelection();
    this->setEnabled(true);
}

#ifdef MAFW
void SinglePlaylistView::closeEvent(QCloseEvent *e)
{
    if (browsePlaylistId != MAFW_SOURCE_INVALID_BROWSE_ID) {
        QString error;
        mafwTrackerSource->cancelBrowse(browsePlaylistId, error);
        if (!error.isEmpty())
            qDebug() << error;
    }

    if (playlistModified && currentObjectId.isNull()) {
        qDebug() << "Playlist modified, saving";
        saveCurrentPlaylist();
    }

    e->accept();
}
#endif
