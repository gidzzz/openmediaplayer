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
    setAttribute(Qt::WA_DeleteOnClose);

    QString shuffleText(tr("Shuffle songs"));
    ui->centralwidget->setLayout(ui->verticalLayout);

#ifdef MAFW
    ui->indicator->setFactory(factory);
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
    shuffleAllButton = new QMaemo5ValueButton(shuffleText, this);
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    shuffleAllButton->setValueText("  songs");
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#else
    shuffleAllButton = new QPushButton(shuffleText, this);
#endif
    SongListItemDelegate *delegate = new SongListItemDelegate(ui->songList);
    ui->songList->setItemDelegate(delegate);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->searchWidget->hide();

    shuffleAllButton->setIcon(QIcon(shuffleButtonIcon));
    ui->verticalLayout->removeWidget(ui->songList);
    ui->verticalLayout->removeWidget(ui->searchWidget);
    ui->verticalLayout->addWidget(shuffleAllButton);
    ui->verticalLayout->addWidget(ui->songList);
    ui->verticalLayout->addWidget(ui->searchWidget);

    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onBrowserContextMenuRequested(QPoint)));
    connect(shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleButtonClicked()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));

    connect(ui->actionEdit_playlist, SIGNAL(triggered()), this, SLOT(enterEditMode()));
    connect(ui->actionSave_playlist, SIGNAL(triggered()), this, SLOT(leaveEditMode()));
    ui->menuOptions->removeAction(ui->actionSave_playlist);

    this->orientationChanged();
}

SinglePlaylistView::~SinglePlaylistView()
{
    delete ui;
}

void SinglePlaylistView::browsePlaylist(MafwPlaylist *mafwplaylist)
{
    qDebug() << "connecting SinglePlaylistView to onGetItems";
    connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
            this, SLOT(onGetItems(QString,GHashTable*,guint,gpointer)));
    this->numberOfSongsToAdd = playlist->getSizeOf(mafwplaylist);
    this->setSongCount(numberOfSongsToAdd);
    browsePlaylistId = (uint)playlist->getItemsOf(mafwplaylist);
}

void SinglePlaylistView::onGetItems(QString objectId, GHashTable* metadata, guint index, gpointer op)
{
    if ((uint)op != browsePlaylistId)
        return;

    qDebug() << "SinglePlaylistView::onGetItems |" << index;
    numberOfSongsToAdd--;
    QString title;
    QString artist;
    QString album;
    int duration = -1;
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(title);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongIndex, index);

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleSongURI, QString::fromUtf8(filename));
            }
        }

        unsigned theIndex = 0;
        int position;
        for (position = 0; position < ui->songList->count(); position++)
        {
            theIndex = ui->songList->item(position)->data(UserRoleSongIndex).toInt();
            if (theIndex > index)
                break;
        }
        ui->songList->insertItem(position, item);
    }

    if (numberOfSongsToAdd == 0) {
        qDebug() << "disconnecting SinglePlaylistView from onGetItems";
        disconnect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
                   this, SLOT(onGetItems(QString,GHashTable*,guint,gpointer)));
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SinglePlaylistView::browseObjectId(QString objectId)
{
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
    ui->menuOptions->removeAction(ui->actionDelete_playlist);
    ui->menuOptions->removeAction(ui->actionEdit_playlist);
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

    QString title;
    QString artist;
    QString album;
    int duration = -1;
    if (metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown artist)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        album = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown album)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        QListWidgetItem *item = new QListWidgetItem(ui->songList);
        item->setText(title);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, duration);

        ui->songList->addItem(item);
    }

#ifdef Q_WS_MAEMO_5
    if (remainingCount == 0)
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    else
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
    this->setSongCount(ui->songList->count());
}

void SinglePlaylistView::onItemSelected(QListWidgetItem *)
{
    disconnect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));

    QString playlistName = playlist->playlistName();
    if (playlistName != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->clear();

    int songCount = ui->songList->count();
    gchar** songAddBuffer = new gchar*[songCount+1];

    for (int i = 0; i < songCount; i++)
        songAddBuffer[i] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[songCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < songCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

#ifdef MAFW
    mafwrenderer->gotoIndex(ui->songList->currentRow());
    playlist->getSize(); // explained in musicwindow.cpp
    mafwrenderer->play();
    mafwrenderer->resume();
#endif

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
    connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));

    window->show();
    window->updatePlaylistState();
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
    QString playlistName = playlist->playlistName();
    if (playlistName != "FmpAudioPlaylist" && playlistName != "FmpVideoPlaylist" && playlistName != "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    int songCount = ui->songList->count();
    gchar** songAddBuffer = new gchar*[songCount+1];

    for (int i = 0; i < songCount; i++)
        songAddBuffer[i] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[songCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < songCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

#ifdef Q_WS_MAEMO_5
    this->notifyOnAddedToNowPlaying(ui->songList->count());
#endif
#endif
}

#ifdef Q_WS_MAEMO_5
void SinglePlaylistView::notifyOnAddedToNowPlaying(int songCount)
{
        QString addedToNp;
        if (songCount == 1)
            addedToNp = tr("clip added to now playing");
        else
            addedToNp = tr("clips added to now playing");
        QMaemo5InformationBox::information(this, QString::number(songCount) + " " + addedToNp);
}
#endif

void SinglePlaylistView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
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

void SinglePlaylistView::onSearchTextChanged(QString text)
{
    for (int i=0; i < ui->songList->count(); i++) {
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
#ifdef MAFW
    QString playlistName = playlist->playlistName();
    if (playlistName != "FmpAudioPlaylist" && playlistName != "FmpVideoPlaylist" && playlistName != "FmpRadioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->clear();
    playlist->setShuffled(true);

    int songCount = ui->songList->count();
    gchar** songAddBuffer = new gchar*[songCount+1];

    for (int i = 0; i < songCount; i++)
        songAddBuffer[i] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[songCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < songCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);

    window->show();
    window->updatePlaylistState();

    uint randomIndex = qrand() % ((playlist->getSize() + 1) - 0) + 0;
    playlist->getSize(); // explained in musicwindow.cpp
    mafwrenderer->gotoIndex(randomIndex);
    mafwrenderer->play();
#endif

}

void SinglePlaylistView::onBrowserContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void SinglePlaylistView::onEditorContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Delete from playlist"), this, SLOT(onDeleteFromPlaylist()));
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
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
#endif
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

    // The code used here (share.(h/cpp/ui) was taken from filebox's source code
    // C) 2010. Matias Perez
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
    this->mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onDeleteUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void SinglePlaylistView::onDeleteUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onDeleteUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QFile song(uri);
    if(song.exists()) {
#ifdef DEBUG
        qDebug() << "Song exists";
#endif
        QMessageBox confirmDelete(QMessageBox::NoIcon,
                                  tr("Delete song?"),
                                  tr("Are you sure you want to delete this song?")+ "\n\n"
                                  + ui->songList->currentItem()->data(UserRoleSongTitle).toString() + "\n"
                                  + ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                                  QMessageBox::Yes | QMessageBox::No,
                                  this);
        confirmDelete.exec();
        if(confirmDelete.result() == QMessageBox::Yes) {
            song.remove();
            ui->songList->removeItemWidget(ui->songList->currentItem());
        }
        else if(confirmDelete.result() == QMessageBox::No)
            ui->songList->clearSelection();
    }
}
#endif

void SinglePlaylistView::setSongCount(int count)
{
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

void SinglePlaylistView::enterEditMode()
{
    qDebug() << "entering playlist edit mode";

    ui->menuOptions->removeAction(ui->actionEdit_playlist);
    ui->menuOptions->insertAction(ui->actionDelete_playlist, ui->actionSave_playlist);

    disconnect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onBrowserContextMenuRequested(QPoint)));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onEditorContextMenuRequested(QPoint)));

    ui->songList->viewport()->setAcceptDrops(true);
    ui->songList->setDragEnabled(true);
    ui->songList->setDragDropMode(QAbstractItemView::InternalMove);
    ui->songList->setAutoScrollMargin(120);
}

void SinglePlaylistView::leaveEditMode()
{
    qDebug() << "leaving playlist edit mode";

    ui->menuOptions->removeAction(ui->actionSave_playlist);
    ui->menuOptions->insertAction(ui->actionDelete_playlist, ui->actionEdit_playlist);

    disconnect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onEditorContextMenuRequested(QPoint)));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onBrowserContextMenuRequested(QPoint)));

    ui->songList->setDragEnabled(false);

    this->saveCurrentPlaylist();
}

void SinglePlaylistView::saveCurrentPlaylist()
{
#ifdef MAFW
    QString playlistName = this->windowTitle();
    int songCount = ui->songList->count();

    MafwPlaylist *targetPlaylist = MAFW_PLAYLIST(playlist->mafw_playlist_manager->createPlaylist(playlistName));
    playlist->clear(targetPlaylist);

    for (int i = 0; i < songCount; i++)
        playlist->appendItem(targetPlaylist, ui->songList->item(i)->data(UserRoleObjectID).toString());
#endif
}

void SinglePlaylistView::onDeleteFromPlaylist()
{
    delete ui->songList->takeItem(ui->songList->currentRow());
    this->setSongCount(ui->songList->count());
}

void SinglePlaylistView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->restore();
    ui->songList->clearSelection();
    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));
}
