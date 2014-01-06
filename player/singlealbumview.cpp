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

#include "singlealbumview.h"

SingleAlbumView::SingleAlbumView(QWidget *parent, MafwAdapterFactory *factory) :
    BrowserWindow(parent, factory)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->objectList->setItemDelegate(new SingleAlbumViewDelegate(ui->objectList));
    ui->objectList->setItemDelegateForRow(0, new ShuffleButtonDelegate(ui->objectList));

    objectProxyModel->setFilterRole(UserRoleSongTitle);

    ui->windowMenu->addAction(tr("Add songs to now playing"), this, SLOT(addAllToNowPlaying()));
    ui->windowMenu->addAction(tr("Delete"                  ), this, SLOT(deleteCurrentAlbum()));

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(updateSongCount()));

#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
#endif
}

void SingleAlbumView::updateSongCount()
{
    objectModel->item(0)->setData(objectProxyModel->rowCount()-1, UserRoleSongCount);
}

#ifdef MAFW
void SingleAlbumView::listSongs()
{
#ifdef DEBUG
    qDebug() << "SingleAlbumView: Source ready";
#endif

    objectModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    objectModel->appendRow(item);

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllSongs(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseAlbumId = mafwTrackerSource->sourceBrowse(albumObjectId.toUtf8(), true, NULL, "+track,+title",
                                                    MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                     MAFW_METADATA_KEY_ALBUM,
                                                                     MAFW_METADATA_KEY_ARTIST,
                                                                     MAFW_METADATA_KEY_DURATION),
                                                    0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleAlbumView::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if (browseId != browseAlbumId) return;

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

        QStandardItem *item = new QStandardItem();
        item->setData(title, UserRoleSongTitle);
        item->setData(artist, UserRoleSongArtist);
        item->setData(album, UserRoleSongAlbum);
        item->setData(objectId, UserRoleObjectID);
        item->setData(duration, UserRoleSongDuration);

        objectModel->appendRow(item);
        updateSongCount();
    }

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllSongs(uint,int,uint,QString,GHashTable*,QString)));
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SingleAlbumView::browseAlbumByObjectId(QString objectId)
{
    this->albumObjectId = objectId;
    if (mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}

void SingleAlbumView::onItemActivated(QModelIndex index)
{
    if (objectModel->rowCount() == 1) return;

    this->setEnabled(false);

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
}

#endif

int SingleAlbumView::appendAllToPlaylist(bool filter)
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

void SingleAlbumView::addAllToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

#ifdef Q_WS_MAEMO_5
    notifyOnAddedToNowPlaying(appendAllToPlaylist(true));
#endif

#endif
}

void SingleAlbumView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().row() <= 0) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(onRingtoneClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void SingleAlbumView::onAddToNowPlaying()
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

void SingleAlbumView::onAddToPlaylist()
{
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
#ifdef MAFW
        playlist->appendItem(picker.playlist, ui->objectList->currentIndex().data(UserRoleObjectID).toString());
#endif
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("%n clip(s) added to playlist", "", 1));
#endif
    }
}

void SingleAlbumView::onRingtoneClicked()
{
    (new RingtoneDialog(this, mafwTrackerSource,
                        ui->objectList->currentIndex().data(UserRoleObjectID).toString(),
                        ui->objectList->currentIndex().data(UserRoleSongTitle).toString(),
                        ui->objectList->currentIndex().data(UserRoleSongArtist).toString()))
    ->show();

    ui->objectList->clearSelection();
}

void SingleAlbumView::onShareClicked()
{
    (new ShareDialog(this, mafwTrackerSource, ui->objectList->currentIndex().data(UserRoleObjectID).toString()))->show();
}

#ifdef MAFW
void SingleAlbumView::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::music")
        listSongs();
}
#endif

void SingleAlbumView::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->objectList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
        objectProxyModel->removeRow(ui->objectList->currentIndex().row());
        updateSongCount();
    }
#endif
    ui->objectList->clearSelection();
}

void SingleAlbumView::deleteCurrentAlbum()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::DeleteAll, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(albumObjectId.toUtf8());
        this->close();
    }
#endif
}

#ifdef Q_WS_MAEMO_5
void SingleAlbumView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void SingleAlbumView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));

    this->onChildClosed();
}
