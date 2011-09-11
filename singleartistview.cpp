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
#include "ui_singleartistview.h"

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

    ThumbnailItemDelegate *delegate = new ThumbnailItemDelegate(ui->albumList);
    ui->albumList->setItemDelegate(delegate);

    ui->albumList->setContextMenuPolicy(Qt::CustomContextMenu);

#ifdef MAFW
    shuffleRequested = false;
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllAlbums(uint, int, uint, QString, GHashTable*, QString)));
    connect(ui->albumList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onAlbumSelected(QListWidgetItem*)));
#endif
    ui->searchWidget->hide();

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));
    connect(ui->actionAdd_songs_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
    connect(ui->albumList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    this->orientationChanged();
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
    ui->albumList->clear();
    QListWidgetItem *shuffleButton = new QListWidgetItem(ui->albumList);
    shuffleButton->setIcon(QIcon(shuffleIcon124));
    shuffleButton->setText(tr("Shuffle songs"));
    shuffleButton->setData(Qt::UserRole, "shuffle");

    this->browseAllAlbumsId = mafwTrackerSource->sourceBrowse(this->artistObjectId.toUtf8(), false, NULL, NULL,
                                                              MAFW_SOURCE_LIST(MAFW_METADATA_KEY_ALBUM,
                                                                               MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                               MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI),
                                                              0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleArtistView::browseAllAlbums(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if(browseId != browseAllAlbumsId)
        return;

    QString albumTitle;
    QString songCount;
    int childcount = -1;
    QString albumArt;
    QListWidgetItem *item = new QListWidgetItem();
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown album)";

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_CHILDCOUNT_1);
        childcount = v ? g_value_get_int(v) : -1;

        if (childcount != -1) {
            songCount.append(QString::number(childcount));
            songCount.append(" ");

            if (childcount == 1)
                songCount.append(tr("song"));
            else
                songCount.append(tr("songs"));
        }

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

    item->setData(UserRoleValueText, songCount);
    item->setData(UserRoleSongCount, childcount);
    item->setData(UserRoleObjectID, objectId);
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

void SingleArtistView::onAlbumSelected(QListWidgetItem *item)
{
#ifdef MAFW
    if (item->data(Qt::UserRole).toString() == "shuffle")
        this->shuffleAllSongs();
    else {
        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        albumView->setAttribute(Qt::WA_DeleteOnClose);
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setWindowTitle(item->text());
        //connect(albumView, SIGNAL(destroyed()), ui->indicator, SLOT(autoSetVisibility()));
        albumView->show();
        //ui->indicator->hide();
    }
    ui->albumList->clearSelection();
#endif
}


void SingleArtistView::orientationChanged()
{
    ui->albumList->setFlow(ui->albumList->flow());
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleArtistView::onSearchTextChanged(QString text)
{
    if (!ui->indicator->isHidden())
        ui->indicator->hide();

    for (int i=1; i < ui->albumList->count(); i++) {
        if (ui->albumList->item(i)->text().toLower().indexOf(text.toLower()) == -1)
            ui->albumList->item(i)->setHidden(true);
        else
            ui->albumList->item(i)->setHidden(false);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->autoSetVisibility();
    }
}

void SingleArtistView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if ((e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) && !ui->searchWidget->isHidden())
        ui->albumList->setFocus();
    else {
        ui->albumList->clearSelection();
        if (ui->searchWidget->isHidden())
            ui->searchWidget->show();
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
}

void SingleArtistView::setSongCount(int songCount)
{
#ifdef MAFW
    this->numberOfSongs = songCount;
#endif
    if (songCount != -1) {
        ui->albumList->item(0)->setData(UserRoleValueText, QString::number(songCount) + " " + tr("songs"));
        ui->albumList->scroll(1, 1);
    }
}

void SingleArtistView::addAllToNowPlaying()
{
    if (ui->albumList->count() > 1) {
#ifdef MAFW

#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

        if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
            playlist->assignAudioPlaylist();

        songAddBufferSize = 0;

        qDebug() << "connecting SingleArtistView to signalSourceBrowseResult";
        connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                this, SLOT(onBrowseAllSongs(uint, int, uint, QString, GHashTable*, QString)));

        this->browseAllAlbumsId = mafwTrackerSource->sourceBrowse(this->artistObjectId.toUtf8(), true, NULL, NULL, 0,
                                                                  0, MAFW_SOURCE_BROWSE_ALL);
#endif
    }
}

#ifdef MAFW
void SingleArtistView::onBrowseAllSongs(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString)
{
    if (this->browseAllAlbumsId != browseId)
        return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    qDebug() << "SingleArtistView::onBrowseAllSongs | index: " << index;
    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    // I was not able to encounter such exception, but I guess better not to remove it yet
    /*if (!objectId.endsWith("/"))
        playlist->appendItem(objectId);*/

    if (remainingCount == 0) {
        playlist->appendItems((const gchar**)songAddBuffer);

        qDebug() << "disconnecting SingleArtistView from signalSourceBrowseResult";
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                   this, SLOT(onBrowseAllSongs(uint, int, uint, QString, GHashTable*, QString)));

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;
        this->browseAllAlbumsId = 0;

        if (shuffleRequested) {
            uint randomIndex = qrand() % ((playlist->getSize() + 1) - 0) + 0;
            mafwrenderer->gotoIndex(randomIndex);
            mafwrenderer->play();

            NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
            //connect(window, SIGNAL(destroyed()), ui->indicator, SLOT(autoSetVisibility()));
            window->onShuffleButtonToggled(true);
            window->show();
            //ui->indicator->hide();
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
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->clear();
    playlist->setShuffled(true);

    shuffleRequested = true;
    this->addAllToNowPlaying();
#endif
}

void SingleArtistView::onContextMenuRequested(const QPoint &point)
{
    if (ui->albumList->currentRow() != 0) {
        QMenu *contextMenu = new QMenu(this);
        contextMenu->setAttribute(Qt::WA_DeleteOnClose);
        contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddAlbumToNowPlaying()));
        contextMenu->addAction(tr("Delete"));
        contextMenu->exec(point);
    }
}

void SingleArtistView::onAddAlbumToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    QString objectIdToBrowse = ui->albumList->currentItem()->data(UserRoleObjectID).toString();
    songAddBufferSize = 0;

    qDebug() << "connecting SingleArtistView to signalSourceBrowseResult";
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onAddAlbumBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

    this->addToNowPlayingId = mafwTrackerSource->sourceBrowse(objectIdToBrowse.toUtf8(), true, NULL, NULL, 0,
                                                              0, MAFW_SOURCE_BROWSE_ALL);
#endif
}

#ifdef MAFW
void SingleArtistView::onAddAlbumBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*, QString)
{
    if (this->addToNowPlayingId != browseId)
        return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    qDebug() << "SingleArtistView::onAddAlbumBrowseResult | index: " << index;
    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        playlist->appendItems((const gchar**)songAddBuffer);

        qDebug() << "disconnecting SingleArtistView from signalSourceBrowseResult";
        connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onAddAlbumBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;
        this->addToNowPlayingId = 0;
#ifdef Q_WS_MAEMO_5
       this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
       this->notifyOnAddedToNowPlaying(songAddBufferSize);
#endif
        songAddBufferSize = 0;
   }
}
#endif

#ifdef Q_WS_MAEMO_5
void SingleArtistView::notifyOnAddedToNowPlaying(int songCount)
{
        QString addedToNp;
        if (songCount == 1)
            addedToNp = tr("clip added to now playing");
        else
            addedToNp = tr("clips added to now playing");
        QMaemo5InformationBox::information(this, QString::number(songCount) + " " + addedToNp);
}
#endif
