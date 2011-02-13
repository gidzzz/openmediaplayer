#include "singlealbumview.h"

SingleAlbumView::SingleAlbumView(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa) :
    QMainWindow(parent),
    ui(new Ui::SingleAlbumView)
#ifdef MAFW
    ,mafwTrackerSource(msa),
    mafwrenderer(mra)
#endif
{
    ui->setupUi(this);
    QString shuffleText(tr("Shuffle songs"));
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
    connect(ui->songList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemSelected(QListWidgetItem*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
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
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllSongs(uint, int, uint, QString, GHashTable*, QString)));

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

void SingleAlbumView::browseAllSongs(uint browseId, int, uint, QString objectId, GHashTable* metadata, QString)
{
    if(browseId != browseAllSongsId)
      return;


    QString title;
    QString artist;
    QString album;
    //QString genre("--");
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

void SingleAlbumView::browseAlbum(QString name)
{
    this->albumName = "localtagfs::music/albums/" + name;
    this->setWindowTitle(name);
    if(mafwTrackerSource->isReady())
        this->listSongs();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listSongs()));
}

void SingleAlbumView::onItemSelected(QListWidgetItem *item)
{
    NowPlayingWindow *npWindow = new NowPlayingWindow(this, this->mafwrenderer, this->mafwTrackerSource);
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
    npWindow->show();
#ifdef Q_WS_MAEMO_5
    mafwrenderer->playObject(item->data(UserRoleObjectID).toString().toAscii());
#endif
    ui->songList->clearSelection();
}

#endif


void SingleAlbumView::orientationChanged()
{
    ui->songList->scroll(1,1);
}
