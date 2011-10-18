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

#include "videoswindow.h"

VideosWindow::VideosWindow(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
    ui(new Ui::VideosWindow)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwTrackerSource(factory->getTrackerSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif

    ThumbnailItemDelegate *delegate = new ThumbnailItemDelegate(ui->listWidget);
    ui->listWidget->setItemDelegate(delegate);

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget->installEventFilter(this);

    sortByActionGroup = new QActionGroup(this);
    sortByActionGroup->setExclusive(true);
    sortByDate = new QAction(tr("Date"), sortByActionGroup);
    sortByDate->setCheckable(true);
    sortByCategory = new QAction(tr("Category"), sortByActionGroup);
    sortByCategory->setCheckable(true);
    this->menuBar()->addActions(sortByActionGroup->actions());
    this->selectView();
    this->connectSignals();
    this->orientationChanged();
#ifdef MAFW
    if (mafwTrackerSource->isReady())
        this->listVideos();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(listVideos()));
#endif
}

VideosWindow::~VideosWindow()
{
    delete ui;
}

void VideosWindow::connectSignals()
{
    connect(ui->listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onVideoSelected(QListWidgetItem*)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->listWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->menubar, SIGNAL(triggered(QAction*)), this, SLOT(onSortingChanged(QAction*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(metadataChanged(QString)), this, SLOT(onSourceMetadataChanged(QString)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString, QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
#endif
}

void VideosWindow::onContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->exec(point);
}

void VideosWindow::onDeleteClicked()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              tr(" "),
                              tr("Delete selected item from device?"),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->listWidget->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->listWidget->currentItem();
    }
#endif
    ui->listWidget->clearSelection();
}

void VideosWindow::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->listWidget->currentItem()->data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void VideosWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->listWidget->currentItem()->data(UserRoleObjectID).toString())
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

void VideosWindow::onVideoSelected(QListWidgetItem *item)
{
    this->setEnabled(false);

#ifdef MAFW
    mafwrenderer->stop(); // prevents the audio playlist from starting after the video ends
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory);
#else
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
    window->showFullScreen();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();

    window->playObject(item->data(UserRoleObjectID).toString());
}

void VideosWindow::onSortingChanged(QAction *action)
{
    if (action == sortByDate) {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        QSettings().setValue("Videos/Sortby", "date");
    } else if (action == sortByCategory) {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        QSettings().setValue("Videos/Sortby", "category");
    }
    this->listVideos();
}

void VideosWindow::selectView()
{
    if (QSettings().value("Videos/Sortby").toString() == "category") {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        sortByCategory->setChecked(true);
    } else {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        sortByDate->setChecked(true);
    }
}

#ifdef MAFW
void VideosWindow::listVideos()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif
    ui->listWidget->clear();

#ifdef DEBUG
    qDebug("Source ready");
#endif
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllVideos(uint, int, uint, QString, GHashTable*, QString)), Qt::UniqueConnection);

    const char *sorting = QSettings().value("Videos/Sortby").toString() == "category" ? "-video-source,+title" : "-added,+title";

    this->browseAllVideosId = mafwTrackerSource->sourceBrowse("localtagfs::videos", false, NULL, sorting,
                                                               MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                                MAFW_METADATA_KEY_DURATION,
                                                                                MAFW_METADATA_KEY_THUMBNAIL_URI,
                                                                                MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI,
                                                                                MAFW_METADATA_KEY_URI,
                                                                                ),
                                                               0, MAFW_SOURCE_BROWSE_ALL);
}

void VideosWindow::browseAllVideos(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString)
{
    if (browseId != browseAllVideosId)
        return;

    if (metadata != NULL) {
        QString title;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown clip)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        if (v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleSongURI, QString::fromUtf8(filename));
            }
        }

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI);
        if (v != NULL) {
            const gchar* filename = g_value_get_string(v); // the uri is really a filename
            if (filename != NULL)
                item->setIcon(QIcon(QString::fromUtf8(filename)));
            else
                item->setIcon(QIcon::fromTheme(defaultVideoIcon));
        } else {
            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_THUMBNAIL_URI);
            if (v != NULL) {
                const gchar* file_uri = g_value_get_string(v); // here uri is a uri
                gchar* filename;
                if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                    item->setIcon(QIcon(QString::fromUtf8(filename)));
                else
                    item->setIcon(QIcon::fromTheme(defaultVideoIcon));
            } else {
                item->setIcon(QIcon::fromTheme(defaultVideoIcon));
            }
        }

        item->setText(title);
        if (duration != Duration::Unknown) {
            QTime t(0, 0);
            t = t.addSecs(duration);
            item->setData(UserRoleValueText, t.toString("h:mm:ss"));
        } else {
            item->setData(UserRoleValueText, "-:--:--");
        }
        item->setData(UserRoleObjectID, objectId);
        ui->listWidget->addItem(item);
    }

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                   this, SLOT(browseAllVideos(uint, int, uint, QString, GHashTable*, QString)));
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void VideosWindow::onMetadataChanged(QString metadata, QVariant value)
{
    QString thumbFile = value.toString();
    QString objectId = ui->indicator->currentObjectId();
    if (metadata == "paused-thumbnail-uri" && objectId.startsWith("localtagfs::videos")) {
        if (thumbFile.contains("mafw-gst-renderer-")) {
            QImage thumbnail(thumbFile);
            if (thumbnail.width() > thumbnail.height()) {
                thumbnail = thumbnail.scaledToHeight(124, Qt::SmoothTransformation);
                thumbnail = thumbnail.copy((thumbnail.width()-124)/2, 0, 124, 124);
            } else {
                thumbnail = thumbnail.scaledToWidth(124, Qt::SmoothTransformation);
                thumbnail = thumbnail.copy(0, (thumbnail.height()-124)/2, 124, 124);
            }

            thumbFile = "/home/user/.fmp_pause_thumbnail/" + QCryptographicHash::hash(objectId.toUtf8(), QCryptographicHash::Md5).toHex() + ".jpeg";
            thumbnail = thumbnail.scaled(124, 124, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            thumbnail.save(thumbFile, "JPEG");

            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI, qstrdup(thumbFile.toUtf8()));
            mafwTrackerSource->setMetadata(objectId.toUtf8(), metadata);
            mafw_metadata_release(metadata);
        }
        this->listVideos();
    }
}

void VideosWindow::onSourceMetadataChanged(QString objectId)
{
    if (objectId.startsWith("localtagfs::videos"))
        this->listVideos();
}

void VideosWindow::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::videos")
        this->listVideos();
}
#endif

void VideosWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

bool VideosWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::Resize)
        ui->listWidget->setFlow(ui->listWidget->flow());
    return false;
}

void VideosWindow::onChildClosed()
{
    ui->indicator->restore();
    ui->listWidget->clearSelection();
    this->setEnabled(true);
}

void VideosWindow::focusInEvent(QFocusEvent *)
{
    ui->indicator->triggerAnimation();
}

void VideosWindow::focusOutEvent(QFocusEvent *)
{
    ui->indicator->stopAnimation();
}
