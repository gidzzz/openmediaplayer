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
    QMainWindow(parent),
    ui(new Ui::SinglePlaylistView)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->verticalLayout);

    setAttribute(Qt::WA_DeleteOnClose);

#ifdef MAFW
    ui->indicator->setFactory(factory);
    browsePlaylistId = MAFW_SOURCE_INVALID_BROWSE_ID;
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif

    permanentDelete = QSettings().value("main/permanentDelete").toBool();

    SongListItemDelegate *songDelegate = new SongListItemDelegate(ui->songList);
    ShuffleButtonDelegate *shuffleDelegate = new ShuffleButtonDelegate(ui->songList);
    ui->songList->setItemDelegate(songDelegate);
    ui->songList->setItemDelegateForRow(0, shuffleDelegate);

    ui->songList->viewport()->installEventFilter(this);

    ui->songList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->songList->viewport()->setAcceptDrops(true);
    ui->songList->setAutoScrollMargin(70);
    QApplication::setStartDragDistance(20);
    ui->songList->setDragEnabled(false);

    playlistModified = false;

    clickedItem = NULL;
    clickTimer = new QTimer(this);
    clickTimer->setInterval(QApplication::doubleClickInterval());
    clickTimer->setSingleShot(true);

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));
    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), ui->windowMenu), SIGNAL(activated()), ui->windowMenu, SLOT(close()));

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
    ui->songList->clear();
    ui->songList->addItem(new QListWidgetItem);
    visibleSongs = 0;

    connect(ui->songList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked()));

    int size = playlist->getSizeOf(mafwplaylist);

    gchar** items = mafw_playlist_get_items(mafwplaylist, 0, size-1, NULL);
    for (int i = 0; items[i] != NULL; i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(UserRoleObjectID, QString::fromUtf8(items[i]));
        item->setData(UserRoleSongDuration, Duration::Blank);
        ui->songList->addItem(item);
    }
    g_strfreev(items);

    playlistQM = new PlaylistQueryManager(this, playlist, mafwplaylist);
    connect(playlistQM, SIGNAL(onGetItems(QString, GHashTable*, guint)), this, SLOT(onGetItems(QString, GHashTable*, guint)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), playlistQM, SLOT(setPriority(int)));
    playlistQM->getItems(0, size-1);

    visibleSongs = remainingCount = size;
    updateSongCount();
}

void SinglePlaylistView::onGetItems(QString objectId, GHashTable* metadata, guint index)
{
    remainingCount--;

    if (playlistModified) {
        for (int i = 1; i < ui->songList->count(); i++)
            if (ui->songList->item(i)->data(UserRoleObjectID).toString() == objectId)
                setItemMetadata(ui->songList->item(i), objectId, metadata);
    } else {
        setItemMetadata(ui->songList->item(index+1), objectId, metadata);
    }

    if (remainingCount == 0) {
        if (!ui->searchEdit->text().isEmpty())
            onSearchTextChanged(ui->searchEdit->text());
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SinglePlaylistView::setItemMetadata(QListWidgetItem *item, QString objectId, GHashTable *metadata)
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
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, objectId);

    } else {
        item->setText(tr("Information not available"));
        item->setData(UserRoleSongDuration, Duration::Blank);
    }
}

void SinglePlaylistView::browseImportedPlaylist(QString objectId)
{
    currentObjectId = objectId;
    ui->songList->clear();
    ui->songList->addItem(new QListWidgetItem);
    visibleSongs = 0;

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);
    browsePlaylistId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                       MAFW_SOURCE_LIST (MAFW_METADATA_KEY_TITLE,
                                                                         MAFW_METADATA_KEY_DURATION,
                                                                         MAFW_METADATA_KEY_ARTIST,
                                                                         MAFW_METADATA_KEY_ALBUM),
                                                       0, MAFW_SOURCE_BROWSE_ALL);
}

void SinglePlaylistView::browseAutomaticPlaylist(QString filter, QString sorting, int maxCount)
{
    currentObjectId = QString();
    ui->songList->clear();
    ui->songList->addItem(new QListWidgetItem);
    visibleSongs = 0;

    ui->windowMenu->removeAction(ui->actionDelete_playlist);

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);
    browsePlaylistId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", true, filter.toUtf8(), sorting.toUtf8(),
                                                       MAFW_SOURCE_LIST (MAFW_METADATA_KEY_TITLE,
                                                                         MAFW_METADATA_KEY_DURATION,
                                                                         MAFW_METADATA_KEY_ARTIST,
                                                                         MAFW_METADATA_KEY_ALBUM),
                                                       0, maxCount);
}

void SinglePlaylistView::onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString)
{
    if (browseId != this->browsePlaylistId) return;

    if (index != 0 || remainingCount != 0 || !objectId.isNull()) {
        QListWidgetItem *item = new QListWidgetItem();
        setItemMetadata(item, objectId, metadata);
        ui->songList->addItem(item);

        ++visibleSongs; updateSongCount();
    }

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));
        if (!ui->searchEdit->text().isEmpty())
            onSearchTextChanged(ui->searchEdit->text());
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SinglePlaylistView::onItemActivated(QListWidgetItem *item)
{
    playAll(ui->songList->row(item));
}

void SinglePlaylistView::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
}

void SinglePlaylistView::addAllToNowPlaying()
{
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
    PlaylistPicker picker(this);
    picker.exec();
    if (picker.result() == QDialog::Accepted) {
        int songCount = ui->songList->count()-1;

        if (currentObjectId.isNull() && picker.playlistName == windowTitle()) {
            for (int i = 0; i < songCount; i++)
                ui->songList->addItem(ui->songList->item(i+1)->clone());
            visibleSongs += songCount; updateSongCount();
            playlistModified = true;
        } else {
#ifdef MAFW
            gchar** songAddBuffer = new gchar*[songCount+1];

            for (int i = 0; i < songCount; i++)
                songAddBuffer[i] = qstrdup(ui->songList->item(i+1)->data(UserRoleObjectID).toString().toUtf8());

            songAddBuffer[songCount] = NULL;

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

void SinglePlaylistView::playAll(int startIndex)
{
#ifdef MAFW
    if (visibleSongs == 0) return;

    this->setEnabled(false);

    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(startIndex < 1);

    bool filter = startIndex == 0 || QSettings().value("main/playlistFilter", false).toBool();

    appendAllToPlaylist(filter);

    if (startIndex > 0) {
        if (filter) {
            int visibleIndex = 0;
            for (int i = 1; i < startIndex; i++)
                if (!ui->songList->item(i)->isHidden())
                   ++visibleIndex;

            mafwrenderer->gotoIndex(visibleIndex);
        } else {
            mafwrenderer->gotoIndex(startIndex-1);
        }
    }

    mafwrenderer->play();

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
    window->show();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();
#endif
}

int SinglePlaylistView::appendAllToPlaylist(bool filter)
{
#ifdef MAFW
    gchar** songAddBuffer = new gchar*[ui->songList->count()];

    int visibleCount;

    if (filter) {
        visibleCount = 0;
        for (int i = 1; i < ui->songList->count(); i++)
            if (!ui->songList->item(i)->isHidden())
                songAddBuffer[visibleCount++] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
    } else {
        visibleCount = ui->songList->count()-1;
        for (int i = 0; i < visibleCount; i++)
            songAddBuffer[i] = qstrdup(ui->songList->item(i+1)->data(UserRoleObjectID).toString().toUtf8());
    }

    songAddBuffer[visibleCount] = NULL;

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
        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_Enter:
            onItemActivated(ui->songList->currentItem());
            break;
    }
}

void SinglePlaylistView::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Backspace:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            return;

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
    visibleSongs = 0;
    for (int i = 1; i < ui->songList->count(); i++) {
        if (ui->songList->item(i)->text().contains(text, Qt::CaseInsensitive)
        || ui->songList->item(i)->data(UserRoleSongArtist).toString().contains(text, Qt::CaseInsensitive)
        || ui->songList->item(i)->data(UserRoleSongAlbum).toString().contains(text, Qt::CaseInsensitive)) {
            ui->songList->item(i)->setHidden(false);
            ++visibleSongs;
        } else
            ui->songList->item(i)->setHidden(true);
    }

    updateSongCount();

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SinglePlaylistView::onContextMenuRequested(const QPoint &pos)
{
    if (ui->songList->currentRow() <= 0) return;

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Add to a playlist"), this, SLOT(onAddToPlaylist()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    if (currentObjectId.isNull()) contextMenu->addAction(tr("Delete from playlist"), this, SLOT(onDeleteFromPlaylist()));
    if (permanentDelete) contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), contextMenu), SIGNAL(activated()), contextMenu, SLOT(close()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void SinglePlaylistView::showWindowMenu()
{
    ui->windowMenu->adjustSize();
    int x = (this->width() - ui->windowMenu->width()) / 2;
    ui->windowMenu->exec(this->mapToGlobal(QPoint(x,-35)));
}

void SinglePlaylistView::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->songList->currentItem()->data(UserRoleObjectID).toString());

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
            ui->songList->addItem(ui->songList->currentItem()->clone());
            ++visibleSongs; updateSongCount();
            playlistModified = true;
        }
#ifdef MAFW
        else
            playlist->appendItem(picker.playlist, ui->songList->currentItem()->data(UserRoleObjectID).toString());
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
                      ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                      ui->songList->currentItem()->text())
        .exec() == QMessageBox::Yes)
    {
        mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
    }
#endif
    ui->songList->clearSelection();
}

#ifdef MAFW
void SinglePlaylistView::onRingingToneUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

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
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void SinglePlaylistView::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QStringList files;
#ifdef DEBUG
    qDebug() << "Sending file:" << uri;
#endif
    files.append(uri);
#ifdef Q_WS_MAEMO_5
    ShareDialog(this, files).exec();
#endif
}
#endif

void SinglePlaylistView::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->songList->currentItem();
        --visibleSongs; updateSongCount();
        playlistModified = true;
    }
#endif
    ui->songList->clearSelection();
}

void SinglePlaylistView::updateSongCount()
{
    ui->songList->item(0)->setData(UserRoleSongCount, visibleSongs);
}

void SinglePlaylistView::forgetClick()
{
    if (clickedItem) onItemActivated(clickedItem);
    ui->songList->setDragEnabled(false);
    clickedItem = NULL;
}

bool SinglePlaylistView::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::Drop) {
        static_cast<QDropEvent*>(e)->setDropAction(Qt::MoveAction);
        playlistModified = true;
    }

    else if (e->type() == QEvent::MouseButtonPress) {
        if (static_cast<QMouseEvent*>(e)->y() > ui->songList->viewport()->height() - 25
        && ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
        clickedItem = ui->songList->itemAt(0, static_cast<QMouseEvent*>(e)->y());
    }

    else if (e->type() == QEvent::MouseButtonRelease) {
        if (clickedItem != ui->songList->currentItem())
            clickedItem = NULL;
        clickTimer->start();
    }

    return false;
}

void SinglePlaylistView::onItemDoubleClicked()
{
    ui->songList->setDragEnabled(true);
    clickedItem = NULL;
    clickTimer->start();
}

void SinglePlaylistView::saveCurrentPlaylist()
{
#ifdef MAFW
    MafwPlaylist *targetPlaylist = MAFW_PLAYLIST(playlist->mafw_playlist_manager->createPlaylist(this->windowTitle()));
    playlist->clear(targetPlaylist);

    for (int i = 1; i < ui->songList->count(); i++)
        playlist->appendItem(targetPlaylist, ui->songList->item(i)->data(UserRoleObjectID).toString());

    playlistModified = false;
#endif
}

void SinglePlaylistView::deletePlaylist()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::DeletePlaylist, this).exec() == QMessageBox::Yes) {
        if (currentObjectId.isNull()) // Saved playlist
            playlist->mafw_playlist_manager->deletePlaylist(this->windowTitle());
        else // Imported playlist
            mafwTrackerSource->destroyObject(currentObjectId.toUtf8());
        this->close();
    }
#endif
}

void SinglePlaylistView::onDeleteFromPlaylist()
{
    delete ui->songList->takeItem(ui->songList->currentRow());
    --visibleSongs; updateSongCount();
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
