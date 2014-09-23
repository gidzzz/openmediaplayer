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

#include "singleartistview.h"

SingleArtistView::SingleArtistView(QWidget *parent, MafwAdapterFactory *factory) :
    BrowserWindow(parent, factory)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylist())
#endif
{
    QFont font; font.setPointSize(13);
    ui->objectList->setFont(font);
    ui->objectList->setAlternatingRowColors(false);
    ui->objectList->setViewMode(QListView::IconMode);
    ui->objectList->setDragDropMode(QAbstractItemView::NoDragDrop);
    ui->objectList->setMovement(QListView::Static);
    ui->objectList->setItemDelegate(new ThumbnailItemDelegate(ui->objectList));

    objectProxyModel->setFilterRole(UserRoleTitle);

    ui->windowMenu->addAction(tr("Add songs to now playing"), this, SLOT(addAllToNowPlaying()));
    ui->windowMenu->addAction(tr("Delete"                  ), this, SLOT(deleteCurrentArtist()));

#ifdef MAFW
    shuffleRequested = false;

    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
#endif

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onAlbumSelected(QModelIndex)));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
}

void SingleArtistView::browseArtist(QString objectId)
{
    artistObjectId = objectId;

    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)), Qt::UniqueConnection);
    if (mafwTrackerSource->isReady())
        listAlbums();
}

#ifdef MAFW
void SingleArtistView::listAlbums()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    objectModel->clear();
    visibleSongs = 0;

    QStandardItem *shuffleButton = new QStandardItem();
    shuffleButton->setIcon(QIcon::fromTheme(defaultShuffleIcon));
    shuffleButton->setData(tr("Shuffle songs"), UserRoleTitle);
    shuffleButton->setData(true, UserRoleHeader);
    objectModel->appendRow(shuffleButton);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllAlbums(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseArtistId = mafwTrackerSource->browse(artistObjectId, false, NULL, NULL,
                                               MAFW_SOURCE_LIST(MAFW_METADATA_KEY_ALBUM,
                                                                MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI),
                                               0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleArtistView::browseAllAlbums(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != browseArtistId) return;

    if (metadata != NULL) {
        QString albumTitle;
        int childcount;
        QString albumArt;
        GValue *v;

        QStandardItem *item = new QStandardItem();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
        childcount = v ? g_value_get_int(v) : 0;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                item->setIcon(QIcon(QString::fromUtf8(filename)));
        } else {
            item->setIcon(QIcon::fromTheme(defaultAlbumIcon));
        }

        item->setData(tr("%n song(s)", "", childcount), UserRoleValueText);
        item->setData(childcount, UserRoleSongCount);
        item->setData(objectId, UserRoleObjectID);
        item->setData(albumTitle, UserRoleTitle);

        objectModel->appendRow(item);
        visibleSongs += childcount; updateSongCount();
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
#endif

void SingleArtistView::onAlbumSelected(QModelIndex index)
{
#ifdef MAFW
    this->setEnabled(false);

    if (index.row() == 0) {
        shuffleAllSongs();
    } else {
        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        albumView->browseAlbumByObjectId(index.data(UserRoleObjectID).toString());
        albumView->setWindowTitle(index.data(UserRoleTitle).toString());
        albumView->show();

        connect(albumView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();
    }
#endif
}

void SingleArtistView::updateSongCount()
{
    objectModel->item(0)->setData(tr("%n song(s)", "", visibleSongs), UserRoleValueText);
}

void SingleArtistView::addAllToNowPlaying()
{
    if (objectModel->rowCount() > 1) {
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

        CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwFactory);
        connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onArtistAddFinished(uint,int)), Qt::UniqueConnection);
        playlistToken = cpm->appendBrowsed(artistObjectId);
    }
}

void SingleArtistView::onArtistAddFinished(uint token, int count)
{
    if (token != playlistToken) return;

    if (shuffleRequested) {
        mafwrenderer->play();

        NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
        window->show();

        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        ui->indicator->inhibit();

        shuffleRequested = false;
    }
#ifdef Q_WS_MAEMO_5
    else {
        notifyOnAddedToNowPlaying(count);
    }

    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}

void SingleArtistView::shuffleAllSongs()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->clear();
    playlist->setShuffled(true);

    shuffleRequested = true;
    this->addAllToNowPlaying();
#endif
}

void SingleArtistView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().row() <= 0) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddAlbumToNowPlaying()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void SingleArtistView::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        QModelIndex index = ui->objectList->currentIndex();
        mafwTrackerSource->destroyObject(index.data(UserRoleObjectID).toString());
        visibleSongs -= index.data(UserRoleSongCount).toInt(); updateSongCount();
        objectProxyModel->removeRow(index.row());
    }
#endif
    ui->objectList->clearSelection();
}

void SingleArtistView::deleteCurrentArtist()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::DeleteAll, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(artistObjectId);
        this->close();
    }
#endif
}

void SingleArtistView::onAddAlbumToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwFactory);
    connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAlbumAddFinished(uint,int)), Qt::UniqueConnection);
    playlistToken = cpm->appendBrowsed(ui->objectList->currentIndex().data(UserRoleObjectID).toString());
}

void SingleArtistView::onAlbumAddFinished(uint token, int count)
{
    if (token != playlistToken) return;

#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    notifyOnAddedToNowPlaying(count);
#endif
}

#ifdef MAFW
void SingleArtistView::onContainerChanged(QString objectId)
{
    if (artistObjectId.startsWith(objectId) || objectId.startsWith(artistObjectId))
        this->listAlbums();
}
#endif

#ifdef Q_WS_MAEMO_5
void SingleArtistView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void SingleArtistView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));

    this->onChildClosed();
}
