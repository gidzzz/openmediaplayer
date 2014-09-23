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
    BrowserWindow(parent, factory)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylist())
#endif
{
#ifdef MAFW
    browsePlaylistId = MAFW_SOURCE_INVALID_BROWSE_ID;
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    permanentDelete = QSettings().value("main/permanentDelete").toBool();

    ui->objectList->setItemDelegate(new SongListItemDelegate(ui->objectList));
    ui->objectList->setItemDelegateForRow(0, new ShuffleButtonDelegate(ui->objectList));

    objectProxyModel->setFilterRole(UserRoleFilterString);

    ui->objectList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->objectList->viewport()->setAcceptDrops(true);
    ui->objectList->setAutoScrollMargin(70);
    QApplication::setStartDragDistance(20);
    ui->objectList->setDragEnabled(false);

    playlistModified = false;
    pendingActivation = Nothing;

    clickedIndex = QModelIndex();
    clickTimer = new QTimer(this);
    clickTimer->setInterval(QApplication::doubleClickInterval());
    clickTimer->setSingleShot(true);

    ui->windowMenu->addAction(tr("Add to now playing"), this, SLOT(addAllToNowPlaying()));
    ui->windowMenu->addAction(tr("Add to a playlist" ), this, SLOT(addAllToPlaylist()));
    ui->windowMenu->addAction(tr("Delete playlist"   ), this, SLOT(deletePlaylist()));

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(updateSongCount()));

    connect(clickTimer, SIGNAL(timeout()), this, SLOT(forgetClick()));
}

void SinglePlaylistView::browseSavedPlaylist(MafwPlaylist *mafwplaylist)
{
    currentObjectId = QString();
    playlistLoaded = true;

    objectModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    item->setDragEnabled(false);
    item->setDropEnabled(false);
    objectModel->appendRow(item);

    connect(ui->objectList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked()));

    int size = playlist->getSizeOf(mafwplaylist);

    gchar** items = mafw_playlist_get_items(mafwplaylist, 0, size-1, NULL);
    for (int i = 0; items[i] != NULL; i++) {
        QStandardItem *item = new QStandardItem();
        item->setData(QString::fromUtf8(items[i]), UserRoleObjectID);
        item->setData(Duration::Blank, UserRoleSongDuration);
        item->setDropEnabled(false);
        objectModel->appendRow(item);
    }
    g_strfreev(items);

    playlistQM = new PlaylistQueryManager(this, playlist, mafwplaylist);
    connect(playlistQM, SIGNAL(onGetItems(QString, GHashTable*, guint)), this, SLOT(onGetItems(QString, GHashTable*, guint)));
    connect(ui->objectList->verticalScrollBar(), SIGNAL(valueChanged(int)), playlistQM, SLOT(setPriority(int)));
    playlistQM->getItems(0, size-1);

    remainingCount = size;
    updateSongCount();
}

void SinglePlaylistView::onGetItems(QString objectId, GHashTable* metadata, guint index)
{
    remainingCount--;

    if (playlistModified) {
        for (int i = 1; i < objectModel->rowCount(); i++)
            if (objectModel->item(i)->data(UserRoleObjectID).toString() == objectId)
                setItemMetadata(objectModel->item(i), objectId, metadata);
    } else {
        setItemMetadata(objectModel->item(index+1), objectId, metadata);
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

    objectModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    objectModel->appendRow(item);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);
    browsePlaylistId = mafwTrackerSource->browse(objectId, true, NULL, NULL,
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

    objectModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    objectModel->appendRow(item);

    // Assume that extended actions are at the end of the menu, in this case the
    // only one should be the delete action.
    ui->windowMenu->removeAction(ui->windowMenu->actions().last());

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);
    browsePlaylistId = mafwTrackerSource->browse("localtagfs::music/songs", true, filter.toUtf8(), sorting.toUtf8(),
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
        objectModel->appendRow(item);
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
                onItemActivated(objectProxyModel->mapFromSource(objectModel->index(pendingActivation,0)));
                break;
        }
    }
}

void SinglePlaylistView::onItemActivated(QModelIndex index)
{
#ifdef MAFW
    this->setEnabled(false);

    if (!playlistLoaded) {
        pendingActivation = objectProxyModel->mapToSource(index).row();
        return;
    }

    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(index.row() == 0);

    bool filter = index.row() == 0 || QSettings().value("main/playlistFilter", false).toBool();

    appendAllToPlaylist(filter);

    mafwrenderer->gotoIndex((filter ? index.row() : objectProxyModel->mapToSource(index).row())-1);
    mafwrenderer->play();

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
    window->show();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();
#endif
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
        int songCount = objectProxyModel->rowCount();

        if (currentObjectId.isNull() && picker.playlistName == windowTitle()) {
            for (int i = 1; i < songCount; i++)
                objectModel->appendRow(objectModel->item(objectProxyModel->mapToSource(objectProxyModel->index(i,0)).row())->clone());
            updateSongCount();
            playlistModified = true;
            --songCount;
        } else {
#ifdef MAFW
            gchar** songAddBuffer = new gchar*[songCount];

            for (int i = 1; i < songCount; i++)
                songAddBuffer[i-1] = qstrdup(objectProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());

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
    int visibleCount = filter ? objectProxyModel->rowCount() : objectModel->rowCount();

    gchar** songAddBuffer = new gchar*[visibleCount];

    if (filter)
        for (int i = 1; i < visibleCount; i++)
            songAddBuffer[i-1] = qstrdup(objectProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());
    else
        for (int i = 1; i < visibleCount; i++)
            songAddBuffer[i-1] = qstrdup(objectModel->item(i)->data(UserRoleObjectID).toString().toUtf8());

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
        case Qt::Key_Enter:
            onItemActivated(ui->objectList->currentIndex());
            break;

        default:
            BrowserWindow::keyPressEvent(e);
            break;
    }
}

void SinglePlaylistView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().row() <= 0) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(onRingtoneClicked()));
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

    playlist->appendItem(ui->objectList->currentIndex().data(UserRoleObjectID).toString());

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
            objectModel->appendRow(objectModel->item(objectProxyModel->mapToSource(ui->objectList->currentIndex()).row())->clone());
            updateSongCount();
            playlistModified = true;
        }
#ifdef MAFW
        else
            playlist->appendItem(picker.playlist, ui->objectList->currentIndex().data(UserRoleObjectID).toString());
#endif
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
#endif
    }
}

void SinglePlaylistView::onRingtoneClicked()
{
    (new RingtoneDialog(this, mafwTrackerSource,
                        ui->objectList->currentIndex().data(UserRoleObjectID).toString(),
                        ui->objectList->currentIndex().data(Qt::DisplayRole).toString(),
                        ui->objectList->currentIndex().data(UserRoleSongArtist).toString()))
    ->show();

    ui->objectList->clearSelection();
}

void SinglePlaylistView::onShareClicked()
{
    (new ShareDialog(this, mafwTrackerSource, ui->objectList->currentIndex().data(UserRoleObjectID).toString()))->show();
}

void SinglePlaylistView::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->objectList->currentIndex().data(UserRoleObjectID).toString());
        objectProxyModel->removeRow(ui->objectList->currentIndex().row());
        updateSongCount();
        playlistModified = true;
    }
#endif
    ui->objectList->clearSelection();
}

void SinglePlaylistView::updateSongCount()
{
    objectModel->item(0)->setData(objectProxyModel->rowCount()-1, UserRoleSongCount);
}

void SinglePlaylistView::forgetClick()
{
    if (clickedIndex.row() != -1) onItemActivated(clickedIndex);
    ui->objectList->setDragEnabled(false);
    clickedIndex = QModelIndex();
}

bool SinglePlaylistView::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
        case QEvent::Drop: {
            QDropEvent *de = static_cast<QDropEvent*>(e);
            if (ui->objectList->indexAt(de->pos()).row() == 0) {
                de->setDropAction(Qt::IgnoreAction);
            } else {
                de->setDropAction(Qt::MoveAction);
                playlistModified = true;
            }
            break;
        }

        case QEvent::MouseButtonPress: {
            clickedIndex = ui->objectList->indexAt(QPoint(0,static_cast<QMouseEvent*>(e)->y()));
            break;
        }

        case QEvent::MouseButtonRelease: {
            if (clickedIndex != ui->objectList->indexAt(QPoint(0,static_cast<QMouseEvent*>(e)->y())))
                clickedIndex = QModelIndex();
            clickTimer->start();
            break;
        }

        default: {
            break;
        }
    }

    return BrowserWindow::eventFilter(obj, e);
}

void SinglePlaylistView::onItemDoubleClicked()
{
    if (ui->objectList->currentIndex().row() != 0) {
        ui->objectList->setDragEnabled(true);
        clickedIndex = QModelIndex();
        clickTimer->start();
    }
}

void SinglePlaylistView::saveCurrentPlaylist()
{
#ifdef MAFW
    MafwPlaylist *targetPlaylist = MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist(this->windowTitle()));
    playlist->clear(targetPlaylist);

    int songCount = objectModel->rowCount();
    gchar** songAddBuffer = new gchar*[songCount];

    for (int i = 1; i < songCount; i++)
        songAddBuffer[i-1] = qstrdup(objectModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
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
            mafwTrackerSource->destroyObject(currentObjectId);
        this->close();
    }
#endif
}

void SinglePlaylistView::onDeleteFromPlaylist()
{
    objectProxyModel->removeRow(ui->objectList->currentIndex().row());
    updateSongCount();
    playlistModified = true;
}

void SinglePlaylistView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));

    this->onChildClosed();
}

#ifdef MAFW
void SinglePlaylistView::closeEvent(QCloseEvent *e)
{
    if (browsePlaylistId != MAFW_SOURCE_INVALID_BROWSE_ID)
        mafwTrackerSource->cancelBrowse(browsePlaylistId);

    if (playlistModified && currentObjectId.isNull()) {
        qDebug() << "Playlist modified, saving";
        saveCurrentPlaylist();
    }

    e->accept();
}
#endif
