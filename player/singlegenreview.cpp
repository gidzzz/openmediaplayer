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

#include "singlegenreview.h"

SingleGenreView::SingleGenreView(QWidget *parent, MafwAdapterFactory *factory) :
    BrowserWindow(parent, factory)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylist())
#endif
{
    ui->objectList->setItemDelegate(new ArtistListItemDelegate(ui->objectList));
    ui->objectList->setItemDelegateForRow(0, new ShuffleButtonDelegate(ui->objectList));

    ui->windowMenu->addAction(tr("Add to now playing"), this, SLOT(addAllToNowPlaying()));

    shuffleRequested = false;

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
}

void SingleGenreView::onItemActivated(QModelIndex index)
{
    if (index.row() == 0) {
        this->setEnabled(false);

        if (playlist->playlistName() != "FmpAudioPlaylist")
            playlist->assignAudioPlaylist();

        playlist->clear();
        playlist->setShuffled(true);
        shuffleRequested = true;

        addAllToNowPlaying();
    }

    else {
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
            artistView->browseArtist(index.data(UserRoleObjectID).toString());
            artistView->setWindowTitle(index.data(Qt::DisplayRole).toString());

            artistView->show();
            connect(artistView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
            ui->indicator->inhibit();
        }
    }
}

#ifdef MAFW
void SingleGenreView::browseGenre(QString objectId)
{
    currentObjectId = objectId;

    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)), Qt::UniqueConnection);
    if (mafwTrackerSource->isReady())
        listArtists();
}

void SingleGenreView::listArtists()
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    visibleSongs = 0;
    objectModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    objectModel->appendRow(item);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllGenres(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseGenreId = mafwTrackerSource->browse(currentObjectId, false, NULL, NULL,
                                              MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                               MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI,
                                                               MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                               MAFW_METADATA_KEY_CHILDCOUNT_2),
                                              0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleGenreView::browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != browseGenreId) return;

    if (metadata != NULL) {
        QString title;
        int songCount = -1;
        int albumCount = -1;
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
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                item->setIcon(QIcon(QString::fromUtf8(filename)));
        } else {
            item->setIcon(QIcon::fromTheme(defaultAlbumIcon));
        }

        if (title.isEmpty()) title = tr("(unknown artist)");

        item->setText(title);
        item->setData(songCount, UserRoleSongCount);
        item->setData(albumCount, UserRoleAlbumCount);
        item->setData(objectId, UserRoleObjectID);

        objectModel->appendRow(item);
        visibleSongs += songCount; updateSongCount();

    }

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllGenres(uint,int,uint,QString,GHashTable*,QString)));
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}
#endif

void SingleGenreView::updateSongCount()
{
    objectModel->item(0)->setData(visibleSongs, UserRoleSongCount);
}

void SingleGenreView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().row() <= 0) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(addArtistToNowPlaying()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void SingleGenreView::addArtistToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    ui->objectList->clearSelection();

    shuffleRequested = false;

    CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwFactory);
    connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAddFinished(uint,int)), Qt::UniqueConnection);
    playlistToken = cpm->appendBrowsed(ui->objectList->currentIndex().data(UserRoleObjectID).toString());
}

void SingleGenreView::onAddFinished(uint token, int count)
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

#ifdef MAFW
void SingleGenreView::onContainerChanged(QString objectId)
{
    if (currentObjectId.startsWith(objectId) || objectId.startsWith(currentObjectId))
        listArtists();
}
#endif

void SingleGenreView::addAllToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    ui->objectList->clearSelection();

    CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwFactory);
    connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAddFinished(uint,int)), Qt::UniqueConnection);
    playlistToken = cpm->appendBrowsed(currentObjectId);
}

#ifdef Q_WS_MAEMO_5
void SingleGenreView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void SingleGenreView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));

    this->onChildClosed();
}
