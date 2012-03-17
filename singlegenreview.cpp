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
    ui->centralwidget->setLayout(ui->verticalLayout);
    setupShuffleButton();

    setAttribute(Qt::WA_DeleteOnClose);

#ifdef MAFW
    ui->indicator->setFactory(factory);
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
    ArtistListItemDelegate *delegate = new ArtistListItemDelegate(ui->artistList);
    ui->artistList->setItemDelegate(delegate);

    ui->artistList->setContextMenuPolicy(Qt::CustomContextMenu);

    this->isShuffling = false;

    connect(ui->artistList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->artistList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->artistList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllGenres(uint, int, uint, QString, GHashTable*, QString)));
#endif

    ui->artistList->viewport()->installEventFilter(this);

    this->orientationChanged();
}

SingleGenreView::~SingleGenreView()
{
    delete ui;
}

void SingleGenreView::setupShuffleButton()
{
#ifdef Q_WS_MAEMO_5
    shuffleButton = new QMaemo5ValueButton();
    shuffleButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
#else
    shuffleButton = new QPushButton();
#endif
    connect(shuffleButton, SIGNAL(clicked()), this, SLOT(onShuffleButtonClicked()));

    shuffleButton->setText(tr("Shuffle songs"));
    shuffleButton->setIcon(QIcon::fromTheme(defaultShuffleIcon));

    ui->artistList->insertItem(0, new QListWidgetItem);
    ui->artistList->setItemWidget(ui->artistList->item(0), shuffleButton);
}

void SingleGenreView::orientationChanged()
{
    ui->artistList->scroll(1,1);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleGenreView::onItemActivated(QListWidgetItem *item)
{
    if (ui->artistList->row(item) == 0) {
        onShuffleButtonClicked();
        return;
    }

    int songCount = item->data(UserRoleAlbumCount).toInt();

    if (songCount == 0 || songCount == 1) {
        this->setEnabled(false);

        SingleAlbumView *albumView = new SingleAlbumView(this, mafwFactory);
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setWindowTitle(item->text());

        albumView->show();
        connect(albumView, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

    } else if (songCount > 1) {
        this->setEnabled(false);

        SingleArtistView *artistView = new SingleArtistView(this, mafwFactory);
        artistView->browseAlbum(item->data(UserRoleObjectID).toString());
        artistView->setWindowTitle(item->text());
        artistView->setSongCount(item->data(UserRoleSongCount).toInt());

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
    if (browseId != this->browseGenreId) return;

    if (metadata != NULL) {
        QString title;
        int songCount = -1;
        int albumCount = -1;
        GValue *v;

        QListWidgetItem *item = new QListWidgetItem();

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
                item->setData(UserRoleAlbumArt, filename);
            }
        }

        if (title.isEmpty()) title = tr("(unknown artist)");

        item->setText(title);
        item->setData(UserRoleSongCount, songCount);
        item->setData(UserRoleAlbumCount, albumCount);
        item->setData(UserRoleObjectID, objectId);

        ui->artistList->addItem(item);
    }

    if (!error.isEmpty())
        qDebug() << error;

    if (remainingCount == 0) {
        if (!ui->searchEdit->text().isEmpty())
            this->onSearchTextChanged(ui->searchEdit->text());
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}
#endif

void SingleGenreView::setSongCount(int count)
{
#ifdef Q_WS_MAEMO_5
    shuffleButton->setValueText(tr("%n song(s)", "", count));
#endif
}

void SingleGenreView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void SingleGenreView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
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
    int visibleSongs = 0;
    for (int i = 1; i < ui->artistList->count(); i++) {
        if (ui->artistList->item(i)->text().contains(text, Qt::CaseInsensitive)) {
            ui->artistList->item(i)->setHidden(false);
            visibleSongs += ui->artistList->item(i)->data(UserRoleSongCount).toInt();
        } else
            ui->artistList->item(i)->setHidden(true);
    }

    setSongCount(visibleSongs);

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
    if (playlist->playlistName() != "FmpAudioPlaylist")
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
    if (playlist->playlistName() != "FmpAudioPlaylist")
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
    if (playlist->playlistName() != "FmpAudioPlaylist")
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
