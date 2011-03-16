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

SingleAlbumView::SingleAlbumView(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa, MafwPlaylistAdapter* pls) :
    QMainWindow(parent),
    ui(new Ui::SingleAlbumView)
#ifdef MAFW
    ,mafwTrackerSource(msa),
    mafwrenderer(mra),
    playlist(pls)
#endif
{
    ui->setupUi(this);
    QString shuffleText(tr("Shuffle songs"));
    ui->centralwidget->setLayout(ui->verticalLayout);

#ifdef MAFW
    ui->indicator->setSources(this->mafwrenderer, this->mafwTrackerSource, this->playlist);
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    shuffleAllButton = new QMaemo5ValueButton(shuffleText, this);
    shuffleAllButton->setValueLayout(QMaemo5ValueButton::ValueUnderTextCentered);
    shuffleAllButton->setValueText("  songs");
#else
    shuffleAllButton = new QPushButton(shuffleText, this);
#endif
    SingleAlbumViewDelegate *delegate = new SingleAlbumViewDelegate(ui->songList);
    ui->songList->setItemDelegate(delegate);
    shuffleAllButton->setIcon(QIcon(shuffleButtonIcon));
    ui->verticalLayout->removeWidget(ui->songList);
    ui->verticalLayout->addWidget(shuffleAllButton);
    ui->verticalLayout->addWidget(ui->songList);
    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));
    connect(shuffleAllButton, SIGNAL(clicked()), this, SLOT(onShuffleButtonClicked()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));
#endif
    this->orientationChanged();
}

SingleAlbumView::~SingleAlbumView()
{
    delete ui;
}

#ifdef MAFW
void SingleAlbumView::listSongs()
{
#ifdef DEBUG
    qDebug() << "MusicWindow: Source ready";
#endif

    ui->songList->clear();

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator);
#endif

    this->browseAllSongsId = mafwTrackerSource->sourceBrowse(this->albumName.toUtf8(), false, NULL, "+track",
                                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_ALBUM,
                                                                              MAFW_METADATA_KEY_ARTIST,
                                                                              MAFW_METADATA_KEY_URI,
                                                                              MAFW_METADATA_KEY_ALBUM_ART_URI,
                                                                              MAFW_METADATA_KEY_DURATION,
                                                                              MAFW_METADATA_KEY_TRACK),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleAlbumView::browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if(browseId != browseAllSongsId)
      return;

    // Sometimes MAFW returns another object ID which ends with / and contains
    // no songs, if this happens, browse songs again with that object ID.
    if (objectId.endsWith("/")) {
        this->albumName = this->albumName + "/";
        this->listSongs();
        return;
    }

    // If we're browsing an artist with a single album, then we only have to list
    // songs from one album, this is returned as the object ID, browse that.
    if (this->isSingleAlbum) {
        this->albumName = objectId;
        // Unset the boolean so this doesn't loop
        this->isSingleAlbum = false;
        this->listSongs();
        return;
    }

    QString title;
    QString artist;
    QString album;
    int duration = -1;
    int trackNumber;
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
        duration = v ? g_value_get_int (v) : -1;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TRACK);
        trackNumber = v ? g_value_get_int (v) : -1;

        QListWidgetItem *item = new QListWidgetItem(ui->songList);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleSongArtist, artist);
        item->setData(UserRoleSongAlbum, album);
        item->setData(UserRoleObjectID, objectId);

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleSongURI, QString::fromUtf8(filename));
            }
        }
        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleAlbumArt, QString::fromUtf8(filename));
            }
        }

        if(duration != -1) {
            QTime t(0,0);
            t = t.addSecs(duration);
            item->setData(UserRoleSongDuration, t.toString("mm:ss"));
            item->setData(UserRoleSongDurationS, duration);
        } else {
            item->setData(UserRoleSongDuration, "--:--");
            item->setData(UserRoleSongDurationS, 0);
        }
        // Although we don't need this to show the song title, we need it to
        // sort alphabatically.
        QString itemTitle;
        itemTitle.append(QString::number(trackNumber));
        itemTitle.append(" ");
        itemTitle.append(title);
        item->setText(itemTitle);
        ui->songList->addItem(item);
    }

#ifdef Q_WS_MAEMO_5
    if (remainingCount == 0)
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    else
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
    QString songCount;
    songCount = QString::number(ui->songList->count());
    songCount.append(" ");
    if(ui->songList->count() != 1)
        songCount.append(tr("songs"));
    else
        songCount.append(tr("song"));
    shuffleAllButton->setValueText(songCount);
#endif
}

#ifdef MAFW
void SingleAlbumView::browseAlbum(QString name)
{
    this->albumName = "localtagfs::music/albums/" + name;
    this->setWindowTitle(name);
    if(mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}

void SingleAlbumView::browseSingleAlbum(QString name)
{
    this->albumName = "localtagfs::music/artists/" + name;
    this->setWindowTitle(name);
    if(mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}

void SingleAlbumView::browseAlbumByObjectId(QString objectId)
{
    this->albumName = objectId;
    if(mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}
#endif

void SingleAlbumView::onItemSelected(QListWidgetItem *item)
{
    npWindow = new NowPlayingWindow(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
    npWindow->setAttribute(Qt::WA_DeleteOnClose);
    npWindow->onSongSelected(ui->songList->currentRow()+1,
                             ui->songList->count(),
                             item->data(UserRoleSongTitle).toString(),
                             item->data(UserRoleSongAlbum).toString(),
                             item->data(UserRoleSongArtist).toString(),
                             item->data(UserRoleSongDurationS).toInt()
                             );
    if(!item->data(UserRoleAlbumArt).isNull())
        npWindow->setAlbumImage(item->data(UserRoleAlbumArt).toString());
    else
        npWindow->setAlbumImage(albumImage);

    this->createPlaylist(false);

    mafwrenderer->gotoIndex(ui->songList->currentRow());
    mafwrenderer->play();
    mafwrenderer->resume();

    ui->songList->clearSelection();
}

#endif


void SingleAlbumView::orientationChanged()
{
    ui->songList->scroll(1,1);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleAlbumView::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Backspace)
        this->close();
}

void SingleAlbumView::onShuffleButtonClicked()
{
    this->createPlaylist(true);
}

void SingleAlbumView::createPlaylist(bool shuffle)
{
#ifdef MAFW
    if (ui->songList->count() > 0) {
#ifdef DEBUG
        qDebug() << "Clearing playlist";
#endif
        playlist->clear();
#ifdef DEBUG
        qDebug() << "Playlist cleared";
#endif
        for (int i = 0; i < ui->songList->count(); i++) {
            QListWidgetItem *item = ui->songList->item(i);
            playlist->appendItem(item->data(UserRoleObjectID).toString());
        }

#ifdef DEBUG
        qDebug() << "Playlist created";
#endif

        if (shuffle) {
            playlist->setShuffled(true);
            npWindow = new NowPlayingWindow(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
            npWindow->setAttribute(Qt::WA_DeleteOnClose);
            npWindow->onShuffleButtonToggled(true);
        }

        npWindow->show();
        mafwrenderer->play();
        mafwrenderer->resume();
    }
#endif
}
