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
    QMainWindow(parent),
    ui(new Ui::SingleGenreView)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
    QString shuffleText(tr("Shuffle songs"));
    ui->centralwidget->setLayout(ui->verticalLayout);

#ifdef MAFW
    ui->indicator->setFactory(factory);
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(shuffleText, this);
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    shuffleAllButton->setValueText(" songs");
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#else
    shuffleAllButton = new QPushButton(shuffleText, this);
#endif
    ArtistListItemDelegate *delegate = new ArtistListItemDelegate(ui->artistList);
    ui->artistList->setItemDelegate(delegate);

    ui->artistList->setContextMenuPolicy(Qt::CustomContextMenu);

    this->isShuffling = false;

    shuffleAllButton->setIcon(QIcon(shuffleButtonIcon));
    ui->verticalLayout->removeWidget(ui->artistList);
    ui->verticalLayout->removeWidget(ui->searchWidget);
    ui->verticalLayout->addWidget(shuffleAllButton);
    ui->verticalLayout->addWidget(ui->artistList);
    ui->verticalLayout->addWidget(ui->searchWidget);

    connect(ui->artistList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));
    connect(ui->artistList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->artistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleButtonClicked()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllGenres(uint, int, uint, QString, GHashTable*, QString)));
#endif
    this->orientationChanged();
}

SingleGenreView::~SingleGenreView()
{
    delete ui;
}

void SingleGenreView::orientationChanged()
{
    ui->artistList->scroll(1,1);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleGenreView::onItemSelected(QListWidgetItem *item)
{
    this->setEnabled(false);

    int songCount = item->data(UserRoleAlbumCount).toInt();
    if(songCount == 0 || songCount == 1) {
        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        if (songCount == 1)
            albumView->isSingleAlbum = true;
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setAttribute(Qt::WA_DeleteOnClose);
        albumView->setWindowTitle(item->data(UserRoleSongName).toString());

        albumView->show();
        connect(albumView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();
    } else if(songCount > 1) {
        SingleArtistView *artistView = new SingleArtistView(this, mafwFactory);
        artistView->browseAlbum(item->data(UserRoleObjectID).toString());
        artistView->setWindowTitle(item->data(UserRoleSongName).toString());
        artistView->setSongCount(item->data(UserRoleSongCount).toInt());
        artistView->setAttribute(Qt::WA_DeleteOnClose);

        artistView->show();
        connect(artistView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();
    }

}

#ifdef MAFW
void SingleGenreView::browseGenre(QString objectId)
{
    objectIdToBrowse = objectId;
    currentObjectId = objectId;
    if (mafwTrackerSource->isReady())
        this->listGenres();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listGenres()));
}

void SingleGenreView::listGenres()
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    this->browseGenreId = mafwTrackerSource->sourceBrowse(this->objectIdToBrowse.toUtf8(), false, NULL, NULL,
                                                          MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                           MAFW_METADATA_KEY_ALBUM_ART_SMALL_URI,
                                                                           MAFW_METADATA_KEY_CHILDCOUNT_1,
                                                                           MAFW_METADATA_KEY_CHILDCOUNT_2),
                                                          0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleGenreView::browseAllGenres(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != this->browseGenreId)
      return;

    QString title;
    int songCount = -1;
    int albumCount = -1;
    QListWidgetItem *item = new QListWidgetItem();
    if (metadata != NULL) {
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
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
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
    item->setData(UserRoleObjectID, objectId);
    ui->artistList->addItem(item);
    if (!error.isEmpty())
        qDebug() << error;

#ifdef Q_WS_MAEMO_5
    if (remainingCount == 0)
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}
#endif

void SingleGenreView::setSongCount(int count)
{
    this->songsInGenre = count;
#ifdef Q_WS_MAEMO_5
    QString songCount;
    songCount = QString::number(count);
    songCount.append(" ");
    if(count != 1)
        songCount.append(tr("songs"));
    else
        songCount.append(tr("song"));
    shuffleAllButton->setValueText(songCount);
#endif
}

void SingleGenreView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if ((e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) && !ui->searchWidget->isHidden())
        ui->artistList->setFocus();
    else {
        ui->artistList->clearSelection();
        if (ui->searchWidget->isHidden())
            ui->indicator->inhibit();
            ui->searchWidget->show();
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
}

void SingleGenreView::onSearchTextChanged(QString text)
{
    for (int i=0; i < ui->artistList->count(); i++) {
        if (ui->artistList->item(i)->text().toLower().indexOf(text.toLower()) == -1)
            ui->artistList->item(i)->setHidden(true);
        else
            ui->artistList->item(i)->setHidden(false);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SingleGenreView::onContextMenuRequested(QPoint point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(addItemToNowPlaying()));
    contextMenu->exec(point);
}

void SingleGenreView::addItemToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    this->objectIdToBrowse = ui->artistList->currentItem()->data(UserRoleObjectID).toString();
    songAddBufferSize = 0;

    ui->artistList->clearSelection();

    this->isShuffling = false;

    qDebug() << "connecting SingleGenreView to signalSourceBrowseResult";
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onNowPlayingBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

    addToNowPlayingId = mafwTrackerSource->sourceBrowse(this->objectIdToBrowse.toUtf8(), true, NULL, NULL, 0, 0, MAFW_SOURCE_BROWSE_ALL);
#endif
}

#ifdef MAFW
void SingleGenreView::onNowPlayingBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable*,QString)
{
    if (this->addToNowPlayingId != browseId)
        return;

    if (songAddBufferSize == 0) {
        songAddBufferSize = remainingCount+1;
        songAddBuffer = new gchar*[songAddBufferSize+1];
        songAddBuffer[songAddBufferSize] = NULL;
    }

    qDebug() << "SingleGenreView::onNowPlayingBrowseResult | index: " << index;
    songAddBuffer[index] = qstrdup(objectId.toUtf8());

    if (remainingCount == 0) {
        playlist->appendItems((const gchar**)songAddBuffer);

        qDebug() << "disconnecting SingleGenreView from signalSourceBrowseResult";
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onNowPlayingBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

        for (int i = 0; i < songAddBufferSize; i++)
            delete[] songAddBuffer[i];
        delete[] songAddBuffer;
        this->addToNowPlayingId = 0;

        if (this->isShuffling) {
            playlist->getSize(); // explained in musicwindow.cpp
            mafwrenderer->play();

            NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);

            window->show();

            connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
            ui->indicator->inhibit();

            this->isShuffling = false;
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

#ifdef MAFW
void SingleGenreView::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::music")
        this->listGenres();
}
#endif

void SingleGenreView::addAllToNowPlaying()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    this->objectIdToBrowse = this->currentObjectId;
    songAddBufferSize = 0;

    ui->artistList->clearSelection();

    qDebug() << "connecting SingleGenreView to signalSourceBrowseResult";
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onNowPlayingBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

    addToNowPlayingId = mafwTrackerSource->sourceBrowse(this->objectIdToBrowse.toUtf8(), true, NULL, NULL, 0, 0, MAFW_SOURCE_BROWSE_ALL);
#endif
}

void SingleGenreView::onShuffleButtonClicked()
{
#ifdef MAFW
    this->setEnabled(false);
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->clear();
    playlist->setShuffled(true);
    this->isShuffling = true;

    this->addAllToNowPlaying();
#endif
}

#ifdef Q_WS_MAEMO_5
void SingleGenreView::notifyOnAddedToNowPlaying(int songCount)
{
        QString addedToNp;
        if (songCount == 1)
            addedToNp = tr("clip added to now playing");
        else
            addedToNp = tr("clips added to now playing");
        QMaemo5InformationBox::information(this, QString::number(songCount) + " " + addedToNp);
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
