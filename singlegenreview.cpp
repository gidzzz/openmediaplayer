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
    BaseWindow(parent),
    ui(new Ui::SingleGenreView)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));

    setAttribute(Qt::WA_DeleteOnClose);

#ifdef MAFW
    ui->indicator->setFactory(factory);
#endif

    ArtistListItemDelegate *artistDelegate = new ArtistListItemDelegate(ui->artistList);
    ui->artistList->setItemDelegate(artistDelegate);
    ShuffleButtonDelegate *shuffleDelegate = new ShuffleButtonDelegate(ui->artistList);
    ui->artistList->setItemDelegateForRow(0, shuffleDelegate);

    artistModel = new QStandardItemModel(this);
    artistProxyModel = new HeaderAwareProxyModel(this);
    artistProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    artistProxyModel->setSourceModel(artistModel);
    ui->artistList->setModel(artistProxyModel);

    shuffleRequested = false;

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->artistList, SIGNAL(activated(QModelIndex)), this, SLOT(onItemActivated(QModelIndex)));
    connect(ui->artistList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->artistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));

    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));

#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
#endif

    ui->artistList->viewport()->installEventFilter(this);

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

SingleGenreView::~SingleGenreView()
{
    delete ui;
}

void SingleGenreView::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
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
    if (mafwTrackerSource->isReady())
        listArtists();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listArtists()));
}

void SingleGenreView::listArtists()
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    visibleSongs = 0;
    artistModel->clear();
    QStandardItem *item = new QStandardItem();
    item->setData(true, UserRoleHeader);
    artistModel->appendRow(item);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllGenres(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseGenreId = mafwTrackerSource->sourceBrowse(currentObjectId.toUtf8(), false, NULL, NULL,
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

        artistModel->appendRow(item);
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
    artistModel->item(0)->setData(visibleSongs, UserRoleSongCount);
}

void SingleGenreView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            break;

        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->artistList->setFocus();
            break;

        default:
            ui->artistList->clearSelection();
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

void SingleGenreView::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->artistList->setFocus();
    }
}

bool SingleGenreView::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress
    && static_cast<QMouseEvent*>(e)->y() > ui->artistList->viewport()->height() - 25
    && ui->searchWidget->isHidden()) {
        ui->indicator->inhibit();
        ui->searchWidget->show();
    }
    return false;
}

void SingleGenreView::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void SingleGenreView::onSearchTextChanged(QString text)
{
    artistProxyModel->setFilterFixedString(text);

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SingleGenreView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->artistList->currentIndex().row() <= 0) return;

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

    ui->artistList->clearSelection();

    shuffleRequested = false;

    CurrentPlaylistManager *cpm = CurrentPlaylistManager::acquire(mafwFactory);
    connect(cpm, SIGNAL(finished(uint,int)), this, SLOT(onAddFinished(uint,int)), Qt::UniqueConnection);
    playlistToken = cpm->appendBrowsed(ui->artistList->currentIndex().data(UserRoleObjectID).toString());
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
    if (objectId == "localtagfs::music")
        listArtists();
}
#endif

void SingleGenreView::addAllToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    ui->artistList->clearSelection();

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

void SingleGenreView::onChildClosed()
{
    ui->indicator->restore();
    ui->artistList->clearSelection();
    this->setEnabled(true);
}
