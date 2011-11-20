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

    ui->objectList->setItemDelegate(new MediaWithIconDelegate(ui->objectList));
    ui->objectList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->objectList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onItemActivated(QListWidgetItem*)));
    connect(ui->objectList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->actionAdd_to_now_playing, SIGNAL(triggered()), this, SLOT(addAllToNowPlaying()));

    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));

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

    ui->objectList->clear();

    connect(mafwSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(onBrowseResult(uint, int, uint, QString, GHashTable*, QString)));

    this->browseId = mafwSource->sourceBrowse(objectId.toUtf8(), true, NULL, NULL,
                                              MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                               MAFW_METADATA_KEY_DURATION,
                                                               MAFW_METADATA_KEY_MIME),
                                              0, MAFW_SOURCE_BROWSE_ALL);
}

void UpnpView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

void UpnpView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if ((e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) && !ui->searchWidget->isHidden())
        ui->objectList->setFocus();
    else {
        ui->objectList->clearSelection();
        if (ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
}

void UpnpView::onSearchTextChanged(QString text)
{
    for (int i = 0; i < ui->objectList->count(); i++) {
        if (ui->objectList->item(i)->data(UserRoleTitle).toString().toLower().indexOf(text.toLower()) == -1)
            ui->objectList->item(i)->setHidden(true);
        else
            ui->objectList->item(i)->setHidden(false);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void UpnpView::onBrowseResult(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if (browseId != this->browseId)
        return;

    if (metadata != NULL) {
        QString title;
        QString mime;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown song)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME);
        mime = v ? QString::fromUtf8(g_value_get_string(v)) : tr("(unknown type)");

        if (mime.startsWith("audio") || mime.startsWith("video")) {
            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
            duration = v ? g_value_get_int(v) : Duration::Unknown;
        } else {
            duration = Duration::Blank;
        }

        QListWidgetItem *item = new QListWidgetItem(ui->objectList);

        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, duration);
        item->setData(UserRoleMIME, mime);
        item->setData(UserRoleTitle, title);

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

        ui->objectList->addItem(item);
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
    if (ui->objectList->currentItem()->data(UserRoleMIME).toString().startsWith("audio")) {
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
        this->setEnabled(false);
        UpnpView *window = new UpnpView(this, mafwFactory, mafwSource);
        window->browseObjectId(objectId);
        window->setWindowTitle(item->data(UserRoleTitle).toString());
        window->show();

        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

    } else if (mime.startsWith("audio")) {
        this->setEnabled(false);
        playlist->assignAudioPlaylist();
        playlist->clear();
        appendAllToPlaylist();

        MafwRendererAdapter *mafwrenderer = mafwFactory->getRenderer();
        playlist->getSize(); // explained in musicwindow.cpp
        mafwrenderer->gotoIndex(ui->objectList->row(item)); // selects a wrong song when the directory contains more than audio
        mafwrenderer->play();

        NowPlayingWindow *window = NowPlayingWindow::acquire(this, mafwFactory);
        window->show();

        connect(window, SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
        ui->indicator->inhibit();

    } else if (mime.startsWith("video")) {
        this->setEnabled(false);
        mafwFactory->getRenderer()->stop(); // prevents the audio playlist from starting after the video ends
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory, mafwSource);
        window->showFullScreen();

        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        ui->indicator->inhibit();

        qDebug() << "attempting to play" << item->data(UserRoleObjectID).toString();
        window->playObject(item->data(UserRoleObjectID).toString());

    } else {
        ui->objectList->clearSelection();
    }
}

void UpnpView::onAddOneToNowPlaying()
{
    if (playlist->playlistName() != "FmpAudioPlaylist")
        playlist->assignAudioPlaylist();

    playlist->appendItem(ui->objectList->currentItem()->data(UserRoleObjectID).toString());

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
    int itemCount = ui->objectList->count();
    gchar** songAddBuffer = new gchar*[itemCount+1];

    int songCount = 0;
    for (int i = 0; i < itemCount; i++)
        if (ui->objectList->item(i)->data(UserRoleMIME).toString().startsWith("audio"))
            songAddBuffer[songCount++] = qstrdup(ui->objectList->item(i)->data(UserRoleObjectID).toString().toUtf8());

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
    QMaemo5InformationBox::information(this, tr("%n clip(s) added to now playing", "", songCount));
#endif
}

void UpnpView::onOrientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void UpnpView::onNowPlayingWindowHidden()
{
    disconnect(NowPlayingWindow::acquire(), SIGNAL(hidden()), this, SLOT(onNowPlayingWindowHidden()));
    ui->indicator->restore();
    ui->objectList->clearSelection();
    this->setEnabled(true);
}

void UpnpView::onChildClosed()
{
    ui->indicator->restore();
    ui->objectList->clearSelection();
    this->setEnabled(true);
}
