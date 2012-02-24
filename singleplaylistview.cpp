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
    setupShuffleButton();

    setAttribute(Qt::WA_DeleteOnClose);

#ifdef MAFW
    ui->indicator->setFactory(factory);
    browsePlaylistId = NULL;
    browsePlaylistOp = NULL;
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif

    SongListItemDelegate *delegate = new SongListItemDelegate(ui->songList);
    ui->songList->setItemDelegate(delegate);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->songList->viewport()->installEventFilter(this);

    ui->songList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->songList->viewport()->setAcceptDrops(true);
    ui->songList->setAutoScrollMargin(70);
    QApplication::setStartDragDistance(20);
    ui->songList->setDragEnabled(false);

    clickedItem = NULL;
    clickTimer = new QTimer(this);
    clickTimer->setInterval(QApplication::doubleClickInterval());
    clickTimer->setSingleShot(true);

    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));

    connect(clickTimer, SIGNAL(timeout()), this, SLOT(forgetClick()));
    connect(ui->songList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked()));
    connect(this, SIGNAL(itemDropped(QListWidgetItem*, int)), this, SLOT(onItemDropped(QListWidgetItem*, int)), Qt::QueuedConnection);
    connect(ui->actionSave_playlist, SIGNAL(triggered()), this, SLOT(saveCurrentPlaylist()));

    this->orientationChanged();
}

SinglePlaylistView::~SinglePlaylistView()
{
    delete ui;
}

void SinglePlaylistView::setupShuffleButton()
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

    ui->songList->insertItem(0, new QListWidgetItem);
    ui->songList->setItemWidget(ui->songList->item(0), shuffleButton);
}

void SinglePlaylistView::browsePlaylist(MafwPlaylist *mafwplaylist)
{
    qDebug() << "connecting SinglePlaylistView to onGetItems";
    connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
            this, SLOT(onGetItems(QString,GHashTable*,guint,gpointer)));

    this->numberOfSongsToAdd = playlist->getSizeOf(mafwplaylist);
    browsePlaylistOp = playlist->getItemsOf(mafwplaylist);
}

void SinglePlaylistView::onGetItems(QString objectId, GHashTable* metadata, guint index, gpointer op)
{
    if (op != browsePlaylistOp)
        return;

    qDebug() << "SinglePlaylistView::onGetItems |" << index;
    numberOfSongsToAdd--;

    QListWidgetItem *item = new QListWidgetItem();

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
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongIndex, index);

    } else {
        item->setText(tr("Information not available"));
        item->setData(UserRoleSongDuration, Duration::Unknown);
    }

    int position;
    for (position = 1; position < ui->songList->count(); position++)
        if (ui->songList->item(position)->data(UserRoleSongIndex).toUInt() > index)
            break;
    ui->songList->insertItem(position, item);

    updateSongCount();

    if (numberOfSongsToAdd == 0) {
        qDebug() << "disconnecting SinglePlaylistView from onGetItems";
        disconnect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
                   this, SLOT(onGetItems(QString,GHashTable*,guint,gpointer)));
        browsePlaylistOp = NULL;
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SinglePlaylistView::browseObjectId(QString objectId)
{
    ui->songList->clear();
    setupShuffleButton();

    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));
    this->browsePlaylistId = mafwTrackerSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                             MAFW_SOURCE_LIST (MAFW_METADATA_KEY_TITLE,
                                                                               MAFW_METADATA_KEY_DURATION,
                                                                               MAFW_METADATA_KEY_ARTIST,
                                                                               MAFW_METADATA_KEY_ALBUM),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void SinglePlaylistView::browseAutomaticPlaylist(QString filter, QString sorting, int maxCount)
{
    ui->songList->clear();
    setupShuffleButton();

    ui->menuOptions->removeAction(ui->actionDelete_playlist);
    ui->menuOptions->removeAction(ui->actionSave_playlist);
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));
    this->browsePlaylistId = mafwTrackerSource->sourceBrowse("localtagfs::music/songs", true, filter.toUtf8(), sorting.toUtf8(),
                                                             MAFW_SOURCE_LIST (MAFW_METADATA_KEY_TITLE,
                                                                               MAFW_METADATA_KEY_DURATION,
                                                                               MAFW_METADATA_KEY_ARTIST,
                                                                               MAFW_METADATA_KEY_ALBUM),
                                                             0, maxCount);
}

void SinglePlaylistView::onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable *metadata, QString)
{
    if (browseId != this->browsePlaylistId)
        return;

    QListWidgetItem *item = new QListWidgetItem();

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
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, duration);
    } else {
        item->setText(tr("Information not available"));
        item->setData(UserRoleSongDuration, Duration::Unknown);
    }

    ui->songList->addItem(item);

    updateSongCount();

    if (remainingCount == 0) {
        browsePlaylistId = NULL;
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SinglePlaylistView::onItemActivated(QListWidgetItem *item)
{
    this->playAll(ui->songList->row(item));
}

void SinglePlaylistView::orientationChanged()
{
    ui->songList->scroll(1,1);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SinglePlaylistView::addAllToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

#ifdef Q_WS_MAEMO_5
    this->notifyOnAddedToNowPlaying(appendAllToPlaylist());
#endif

#endif
}

void SinglePlaylistView::playAll(int startIndex)
{
#ifdef MAFW
    this->setEnabled(false);

    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(startIndex < 1);

    appendAllToPlaylist();

    playlist->getSize(); // explained in musicwindow.cpp

    if (startIndex > 0) {
        int visibleIndex = 0;
        for (int i = 1; i < startIndex; i++)
            if (!ui->songList->item(i)->isHidden())
               ++visibleIndex;

        mafwrenderer->gotoIndex(visibleIndex);
    }

    mafwrenderer->play();

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);

    window->show();

    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->inhibit();
#endif
}

int SinglePlaylistView::appendAllToPlaylist()
{
#ifdef MAFW
    gchar** songAddBuffer = new gchar*[ui->songList->count()];

    int visibleCount = 0;
    for (int i = 1; i < ui->songList->count(); i++)
        if (!ui->songList->item(i)->isHidden())
            songAddBuffer[visibleCount++] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
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
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void SinglePlaylistView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if (e->key() == Qt::Key_Enter)
        onItemActivated(ui->songList->currentItem());
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
        ui->songList->setFocus();
    else {
        ui->songList->clearSelection();
        if (ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
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
    for (int i = 1; i < ui->songList->count(); i++) {
        if (ui->songList->item(i)->text().toLower().indexOf(text.toLower()) != -1 ||
            ui->songList->item(i)->data(UserRoleSongArtist).toString().toLower().indexOf(text.toLower()) != -1 ||
            ui->songList->item(i)->data(UserRoleSongAlbum).toString().toLower().indexOf(text.toLower()) != -1)
            ui->songList->item(i)->setHidden(false);
        else
            ui->songList->item(i)->setHidden(true);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void SinglePlaylistView::onShuffleButtonClicked()
{
    this->playAll(0);
}

void SinglePlaylistView::onContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    contextMenu->addAction(tr("Delete from playlist"), this, SLOT(onDeleteFromPlaylist()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void SinglePlaylistView::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() == "FmpVideoPlaylist" || playlist->playlistName() == "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->songList->currentItem()->data(UserRoleObjectID).toString());

#ifdef Q_WS_MAEMO_5
    this->notifyOnAddedToNowPlaying(ui->songList->selectedItems().count());
#endif

#endif
}

void SinglePlaylistView::setRingingTone()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Are you sure you want to set this song as ringing tone?")+ "\n\n"
                              + ui->songList->currentItem()->data(UserRoleSongTitle).toString() + "\n"
                              + ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
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
    QMaemo5InformationBox::information(this, "Selected song set as ringing tone");
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

    QStringList list;
    QString clip;
    clip = uri;
#ifdef DEBUG
    qDebug() << "Sending file:" << clip;
#endif
    list.append(clip);
#ifdef Q_WS_MAEMO_5
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
#endif
}
#endif

void SinglePlaylistView::onDeleteClicked()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              tr("Delete song?"),
                              tr("Are you sure you want to delete this song?")+ "\n\n"
                              + ui->songList->currentItem()->data(UserRoleSongTitle).toString() + "\n"
                              + ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->songList->currentItem();
    }
#endif
    updateSongCount();
    ui->songList->clearSelection();
}

void SinglePlaylistView::updateSongCount()
{
#ifdef Q_WS_MAEMO_5
    shuffleButton->setValueText(tr("%n song(s)", "", ui->songList->count()-1));
#endif
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
        emit itemDropped(ui->songList->currentItem(), ui->songList->currentRow());
    }

    else if (e->type() == QEvent::MouseButtonPress) {
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

void SinglePlaylistView::onItemDropped(QListWidgetItem *item, int from)
{
    int to = ui->songList->row(item);
    if (to == 0) ui->songList->insertItem(++to, ui->songList->takeItem(0));
#ifdef MAFW
    playlist->moveItem(from-1, to-1);
#endif
}

void SinglePlaylistView::saveCurrentPlaylist()
{
#ifdef MAFW
    QString playlistName = this->windowTitle();
    int songCount = ui->songList->count();

    MafwPlaylist *targetPlaylist = MAFW_PLAYLIST(playlist->mafw_playlist_manager->createPlaylist(playlistName));
    playlist->clear(targetPlaylist);

    for (int i = 1; i < songCount; i++)
        playlist->appendItem(targetPlaylist, ui->songList->item(i)->data(UserRoleObjectID).toString());
#endif
}

void SinglePlaylistView::onDeleteFromPlaylist()
{
    delete ui->songList->takeItem(ui->songList->currentRow());
    updateSongCount();
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
    if (browsePlaylistId) {
        QString error;
        mafwTrackerSource->cancelBrowse(browsePlaylistId, error);
    }
    if (browsePlaylistOp)
        mafw_playlist_cancel_get_items_md(browsePlaylistOp);

    e->accept();
}
#endif
