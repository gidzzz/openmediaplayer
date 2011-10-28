#include "upnpview.h"

UpnpView::UpnpView(QWidget *parent, MafwAdapterFactory *factory, MafwSourceAdapter *source) :
    QMainWindow(parent),
    ui(new Ui::UpnpView),
    mafwFactory(factory),
    mafwSource(source),
    playlist(factory->getPlaylistAdapter())
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->verticalLayout);

#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->songList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->songList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->songList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->songList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(onOrientationChanged()));

    ui->indicator->setFactory(factory);

    this->onOrientationChanged();
}

UpnpView::~UpnpView()
{
    delete ui;
}

void UpnpView::browseObjectId(QString objectId)
{
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    ui->songList->clear();

    connect(mafwSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(onBrowseResult(uint, int, uint, QString, GHashTable*, QString)));

    this->browseId = mafwSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                              MAFW_METADATA_KEY_MIME),
                                                             0, MAFW_SOURCE_BROWSE_ALL);
}

void UpnpView::onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if (metadata != NULL) {
        QString title;
        QString mime;
        GValue *v;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_MIME);
        mime = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown mime)");

        QListWidgetItem *item = new QListWidgetItem(ui->songList);

        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleMIME, mime);
        item->setText(title + " (" + mime + ")");

        if (mime == "x-mafw/container")
            item->setIcon(QIcon::fromTheme("general_folder"));
        else if (mime.startsWith("audio"))
            item->setIcon(QIcon::fromTheme("general_audio_file"));
        else if (mime.startsWith("video"))
            item->setIcon(QIcon::fromTheme("general_video_file"));
        else if (mime.startsWith("image"))
            item->setIcon(QIcon::fromTheme("general_image"));
        else
            item->setIcon(QIcon::fromTheme("filemanager_unknown_file"));

        ui->songList->addItem(item);
    }

    if (remainingCount == 0) {
        disconnect(mafwSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                   this, SLOT(onBrowseResult(uint, int, uint, QString, GHashTable*, QString)));
#ifdef Q_WS_MAEMO_5
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void UpnpView::onContextMenuRequested(const QPoint &point)
{
    if (ui->songList->currentItem()->data(UserRoleMIME).toString().startsWith("audio")) {
        QMenu *contextMenu = new QMenu(this);
        contextMenu->setAttribute(Qt::WA_DeleteOnClose);
        contextMenu->addAction(tr("Add to now playing"), this, SLOT(onAddOneToNowPlaying()));
        contextMenu->exec(point);
    }
}

void UpnpView::onItemActivated(QListWidgetItem *item)
{
    QString objectId = item->data(UserRoleObjectID).toString();
    QString mime = item->data(UserRoleMIME).toString();

    if (mime == "x-mafw/container") {
        UpnpView *window = new UpnpView(this, mafwFactory, mafwSource);
        window->browseObjectId(objectId);
        window->show();
    } else {
        playlist->assignAudioPlaylist();
        playlist->clear();
        appendAllToPlaylist();

        MafwRendererAdapter *mafwrenderer = mafwFactory->getRenderer();
        playlist->getSize(); // explained in musicwindow.cpp
        mafwrenderer->gotoIndex(ui->songList->row(item)); // selects a wrong song when the directory contains more than audio
        mafwrenderer->play();

        NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
        window->show();

        //connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        //ui->indicator->inhibit();
    }
}

void UpnpView::onAddOneToNowPlaying()
{
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->songList->currentItem()->data(UserRoleObjectID).toString());

    notifyOnAddedToNowPlaying(1);
}

void UpnpView::addAllToNowPlaying()
{
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    notifyOnAddedToNowPlaying(appendAllToPlaylist());
}

int UpnpView::appendAllToPlaylist()
{
    int itemCount = ui->songList->count();
    gchar** songAddBuffer = new gchar*[itemCount+1];

    int songCount = 0;
    for (int i = 0; i < itemCount; i++)
        if (ui->songList->item(i)->data(UserRoleMIME).toString().startsWith("audio"))
            songAddBuffer[songCount++] = qstrdup(ui->songList->item(i)->data(UserRoleObjectID).toString().toUtf8());

    songAddBuffer[songCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < songCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    return songCount;
}

void UpnpView::notifyOnAddedToNowPlaying(int songCount)
{
#ifdef Q_WS_MAEMO_5
        QString addedToNp;
        if (songCount == 1)
            addedToNp = tr("clip added to now playing");
        else
            addedToNp = tr("clips added to now playing");
        QMaemo5InformationBox::information(this, QString::number(songCount) + " " + addedToNp);
#endif
}

void UpnpView::onOrientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}
