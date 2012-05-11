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

#include "singlealbumview.h"

SingleAlbumView::SingleAlbumView(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
    ui(new Ui::SingleAlbumView)
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
    ui->indicator->setFactory(mafwFactory);
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif

    SingleAlbumViewDelegate *delegate = new SingleAlbumViewDelegate(ui->songList);
    ui->songList->setItemDelegate(delegate);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(deleteCurrentAlbum()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));
#endif

    ui->songList->viewport()->installEventFilter(this);

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

SingleAlbumView::~SingleAlbumView()
{
    delete ui;
}

void SingleAlbumView::setupShuffleButton()
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

void SingleAlbumView::updateSongCount()
{
#ifdef Q_WS_MAEMO_5
    shuffleButton->setValueText(tr("%n song(s)", "", visibleSongs));
#endif
}

#ifdef MAFW
void SingleAlbumView::listSongs()
{
#ifdef DEBUG
    qDebug() << "SingleAlbumView: Source ready";
#endif

    ui->songList->clear();
    visibleSongs = 0;
    setupShuffleButton();

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator);
#endif

    this->browseAllSongsId = mafwTrackerSource->sourceBrowse(this->albumObjectId.toUtf8(), true, NULL, "+track,+title",
                                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_ALBUM,
                                                                              MAFW_METADATA_KEY_ARTIST,
                                                                              MAFW_METADATA_KEY_DURATION),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleAlbumView::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if (browseId != browseAllSongsId) return;

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

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, duration);

        ++visibleSongs; updateSongCount();

        ui->songList->addItem(item);
    }

    if (remainingCount == 0) {
        if (!ui->searchEdit->text().isEmpty())
            this->onSearchTextChanged(ui->searchEdit->text());
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void SingleAlbumView::browseAlbumByObjectId(QString objectId)
{
    this->albumObjectId = objectId;
    if (mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}

void SingleAlbumView::onItemActivated(QListWidgetItem *item)
{
    this->playAll(ui->songList->row(item), QSettings().value("main/playlistFilter", false).toBool());
}

#endif


void SingleAlbumView::orientationChanged(int w, int h)
{
    ui->songList->scroll(1,1);

    ui->indicator->setGeometry(w-122, h-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleAlbumView::onShuffleButtonClicked()
{
    this->playAll(0, true);
}

void SingleAlbumView::playAll(int startIndex, bool filter)
{
#ifdef MAFW
    if (visibleSongs == 0) return;

    this->setEnabled(false);

    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();
    playlist->clear();
    playlist->setShuffled(startIndex < 1);

    appendAllToPlaylist(filter);

    playlist->getSize(); // explained in musicwindow.cpp

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

int SingleAlbumView::appendAllToPlaylist(bool filter)
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

void SingleAlbumView::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void SingleAlbumView::onSearchTextChanged(QString text)
{
    visibleSongs = 0;
    for (int i = 1; i < ui->songList->count(); i++) {
        if (ui->songList->item(i)->data(UserRoleSongTitle).toString().contains(text, Qt::CaseInsensitive)) {
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

void SingleAlbumView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void SingleAlbumView::keyReleaseEvent(QKeyEvent *e)
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

bool SingleAlbumView::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress
    && static_cast<QMouseEvent*>(e)->y() > ui->songList->viewport()->height() - 25
    && ui->searchWidget->isHidden()) {
        ui->indicator->inhibit();
        ui->searchWidget->show();
    }
    return false;
}

void SingleAlbumView::addAllToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

#ifdef Q_WS_MAEMO_5
    this->notifyOnAddedToNowPlaying(appendAllToPlaylist(true));
#endif

#endif
}

void SingleAlbumView::onContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddToNowPlaying()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Set as ringing tone"), this, SLOT(setRingingTone()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void SingleAlbumView::onAddToNowPlaying()
{
#ifdef MAFW
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->songList->currentItem()->data(UserRoleObjectID).toString());

#ifdef Q_WS_MAEMO_5
    this->notifyOnAddedToNowPlaying(1);
#endif

#endif
}

void SingleAlbumView::setRingingTone()
{
#ifdef MAFW
    QMessageBox confirmRingtone(QMessageBox::NoIcon,
                              " ",
                              tr("Are you sure you want to set this song as ringing tone?")+ "\n\n"
                              + ui->songList->currentItem()->data(UserRoleSongTitle).toString() + "\n"
                              + ui->songList->currentItem()->data(UserRoleSongArtist).toString(),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                              this);
    confirmRingtone.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmRingtone.button(QMessageBox::No)->setText(tr("No"));
    confirmRingtone.exec();
    if (confirmRingtone.result() == QMessageBox::Yes) {
        mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onRingingToneUriReceived(QString,QString)));
    }
#endif
    ui->songList->clearSelection();
}

#ifdef MAFW
void SingleAlbumView::onRingingToneUriReceived(QString objectId, QString uri)
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

void SingleAlbumView::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void SingleAlbumView::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->songList->currentItem()->data(UserRoleObjectID).toString())
        return;

    QStringList list;
    QString clip;
    clip = uri;

    list.append(clip);
#ifdef Q_WS_MAEMO_5
    Share *share = new Share(this, list);
    share->setAttribute(Qt::WA_DeleteOnClose);
    share->show();
#endif
}
#endif

#ifdef MAFW
void SingleAlbumView::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::music")
        this->listSongs();
}
#endif

void SingleAlbumView::onDeleteClicked()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Delete selected item from device?"),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->songList->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->songList->currentItem();
        --visibleSongs; updateSongCount();
    }
#endif
    ui->songList->clearSelection();
}

void SingleAlbumView::deleteCurrentAlbum()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Delete all items shown in view?"),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(albumObjectId.toUtf8());
        this->close();
    }
#endif
}

#ifdef Q_WS_MAEMO_5
void SingleAlbumView::notifyOnAddedToNowPlaying(int songCount)
{
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
}
#endif

void SingleAlbumView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->restore();
    ui->songList->clearSelection();
    this->setEnabled(true);
}
