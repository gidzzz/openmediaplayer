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
    QMainWindow(parent),
    ui(new Ui::SingleArtistView)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->verticalLayout);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    setAttribute(Qt::WA_DeleteOnClose);

    ThumbnailItemDelegate *delegate = new ThumbnailItemDelegate(ui->albumList);
    ui->albumList->setItemDelegate(delegate);

#ifdef MAFW
    shuffleRequested = false;

    ui->indicator->setFactory(mafwFactory);

    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
#endif

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));
    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), ui->windowMenu), SIGNAL(activated()), ui->windowMenu, SLOT(close()));

    connect(ui->albumList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onAlbumSelected(QListWidgetItem*)));
    connect(ui->albumList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->albumList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));

    connect(ui->actionAdd_songs_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(deleteCurrentArtist()));

    ui->albumList->viewport()->installEventFilter(this);

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

SingleArtistView::~SingleArtistView()
{
    delete ui;
}

void SingleArtistView::browseAlbum(QString album)
{
#ifdef MAFW
    this->artistObjectId = album;
    this->listAlbums();
#else
    Q_UNUSED(album)
#endif
}

#ifdef MAFW
void SingleArtistView::listAlbums()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    ui->albumList->clear();
    visibleSongs = 0;

    QListWidgetItem *shuffleButton = new QListWidgetItem(ui->albumList);
    shuffleButton->setIcon(QIcon::fromTheme(defaultShuffleIcon));
    shuffleButton->setData(UserRoleTitle, tr("Shuffle songs"));

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllAlbums(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseAllAlbumsId = mafwTrackerSource->sourceBrowse(artistObjectId.toUtf8(), false, NULL, NULL,
                                                        MAFW_SOURCE_LIST(MAFW_METADATA_KEY_ALBUM,
                                                                         MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                         MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI),
                                                        0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleArtistView::browseAllAlbums(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != browseAllAlbumsId) return;

    if (metadata != NULL) {
        QString albumTitle;
        int childcount;
        QString albumArt;
        GValue *v;

        QListWidgetItem *item = new QListWidgetItem();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_CHILDCOUNT_1);
        childcount = v ? g_value_get_int(v) : 0;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setIcon(QIcon(QString::fromUtf8(filename)));
            }
        } else {
            item->setIcon(QIcon::fromTheme(defaultAlbumIcon));
        }

        item->setData(UserRoleValueText, tr("%n song(s)", "", childcount));
        item->setData(UserRoleSongCount, childcount);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleTitle, albumTitle);

        visibleSongs += childcount; updateSongCount();

        ui->albumList->addItem(item);
    }

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllAlbums(uint,int,uint,QString,GHashTable*,QString)));
        if (!ui->searchEdit->text().isEmpty())
            onSearchTextChanged(ui->searchEdit->text());
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}
#endif

void SingleArtistView::onAlbumSelected(QListWidgetItem *item)
{
#ifdef MAFW
    this->setEnabled(false);

    if (ui->albumList->row(item) == 0) {
        shuffleAllSongs();
    } else {
        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setWindowTitle(item->data(UserRoleTitle).toString());
        albumView->show();

        connect(albumView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();
    }
#endif
}

void SingleArtistView::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
}

bool SingleArtistView::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::Resize)
        ui->albumList->setFlow(ui->albumList->flow());
    else
        if (e->type() == QEvent::MouseButtonPress
        && static_cast<QMouseEvent*>(e)->y() > ui->albumList->viewport()->height() - 25
        && ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
    return false;
}

void SingleArtistView::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void SingleArtistView::onSearchTextChanged(QString text)
{
    for (int i = 1; i < ui->albumList->count(); i++) {
        if (ui->albumList->item(i)->data(UserRoleTitle).toString().contains(text, Qt::CaseInsensitive))
            ui->albumList->item(i)->setHidden(false);
        else
            ui->albumList->item(i)->setHidden(true);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SingleArtistView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void SingleArtistView::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Backspace:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            return;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            ui->albumList->setFocus();
            break;

        default:
            ui->albumList->clearSelection();
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

void SingleArtistView::updateSongCount()
{
    ui->albumList->item(0)->setData(UserRoleValueText, tr("%n song(s)", "", visibleSongs));
}

void SingleArtistView::addAllToNowPlaying()
{
    if (ui->albumList->count() > 1) {
#ifdef MAFW

#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

        if (playlist->playlistName() != "FmpAudioPlaylist")
            playlist->assignAudioPlaylist();

        songAddBufferSize = 0;

        connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                this, SLOT(onBrowseAllSongs(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

        browseAllAlbumsId = mafwTrackerSource->sourceBrowse(artistObjectId.toUtf8(), true, NULL, NULL, 0,
                                                            0, MAFW_SOURCE_BROWSE_ALL);
#endif
    }
}

#ifdef MAFW
void SingleArtistView::onBrowseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString)
{
    if (this->browseAllAlbumsId != browseId) return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onBrowseAllSongs(uint,int,uint,QString,GHashTable*,QString)));

        playlist->appendItems((const gchar**)songAddBuffer);

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

        if (shuffleRequested) {
            mafwrenderer->play();

            NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
            window->onShuffleButtonToggled(true);
            window->show();

            connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
            ui->indicator->inhibit();

            shuffleRequested = false;
        }
#ifdef Q_WS_MAEMO_5
        else {
            this->notifyOnAddedToNowPlaying(songAddBufferSize);
        }

        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
        songAddBufferSize = 0;
    }
}
#endif

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
    if (ui->albumList->currentRow() <= 0) return;

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddAlbumToNowPlaying()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), contextMenu), SIGNAL(activated()), contextMenu, SLOT(close()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void SingleArtistView::showWindowMenu()
{
    ui->windowMenu->adjustSize();
    int x = (this->width() - ui->windowMenu->width()) / 2;
    ui->windowMenu->exec(this->mapToGlobal(QPoint(x,-35)));
}

void SingleArtistView::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        QListWidgetItem *item = ui->albumList->currentItem();
        mafwTrackerSource->destroyObject(item->data(UserRoleObjectID).toString().toUtf8());
        visibleSongs -= item->data(UserRoleSongCount).toInt(); updateSongCount();
        delete item;
    }
#endif
    ui->albumList->clearSelection();
}

void SingleArtistView::deleteCurrentArtist()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::DeleteAll, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(artistObjectId.toUtf8());
        this->close();
    }
#endif
}

void SingleArtistView::onAddAlbumToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    QString objectIdToBrowse = ui->albumList->currentItem()->data(UserRoleObjectID).toString();
    songAddBufferSize = 0;

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onAddAlbumBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    addToNowPlayingId = mafwTrackerSource->sourceBrowse(objectIdToBrowse.toUtf8(), true, NULL, NULL, 0,
                                                        0, MAFW_SOURCE_BROWSE_ALL);
#endif
}

#ifdef MAFW
void SingleArtistView::onAddAlbumBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString)
{
    if (browseId != this->addToNowPlayingId) return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onAddAlbumBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

        playlist->appendItems((const gchar**)songAddBuffer);

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;

#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
        this->notifyOnAddedToNowPlaying(songAddBufferSize);
#endif
        songAddBufferSize = 0;
   }
}
#endif

#ifdef MAFW
void SingleArtistView::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::music")
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

void SingleArtistView::onChildClosed()
{
    ui->indicator->restore();
    ui->albumList->clearSelection();
    this->setEnabled(true);
}
