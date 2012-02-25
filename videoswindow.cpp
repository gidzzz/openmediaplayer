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
    this->connectSignals();
    this->orientationChanged();
#ifdef MAFW
    if (mafwTrackerSource->isReady())
        this->selectView();
    else
        connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(selectView()));
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
    if (!ui->listWidget->currentItem()->data(UserRoleObjectID).toString().isEmpty()) {
        QMenu *contextMenu = new QMenu(this);
        contextMenu->setAttribute(Qt::WA_DeleteOnClose);
        contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
        contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
        contextMenu->exec(point);
    }
}

void VideosWindow::onDeleteClicked()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Delete selected item from device?"),
                              QMessageBox::Yes | QMessageBox::No,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
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
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory, mafwTrackerSource);
#else
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
    window->showFullScreen();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();

    playlist->assignVideoPlaylist();
    playlist->clear();

    int videoCount = ui->listWidget->count();
    gchar** videoAddBuffer = new gchar*[videoCount+1];

    for (int i = 0; i < videoCount; i++)
        videoAddBuffer[i] = qstrdup(ui->listWidget->item(i)->data(UserRoleObjectID).toString().toUtf8());
    videoAddBuffer[videoCount] = NULL;

    playlist->appendItems((const gchar**)videoAddBuffer);

    for (int i = 0; i < videoCount; i++)
        delete[] videoAddBuffer[i];
    delete[] videoAddBuffer;

    playlist->getSize(); // explained in musicwindow.cpp
    mafwrenderer->gotoIndex(ui->listWidget->row(item));

    QTimer::singleShot(500, window, SLOT(playVideo()));
}

void VideosWindow::onSortingChanged(QAction *action)
{
    if (action == sortByDate) {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        QSettings().setValue("Videos/Sortby", "date");

        QFont font; font.setPointSize(13); ui->listWidget->setFont(font);
        ui->listWidget->setAlternatingRowColors(false);
        ui->listWidget->setViewMode(QListView::IconMode);
        ui->listWidget->setItemDelegate(new ThumbnailItemDelegate(ui->listWidget));
        ui->listWidget->setWrapping(true);
        ui->listWidget->setGridSize(QSize(155,215));
        ui->listWidget->setFlow(QListView::LeftToRight);

    } else if (action == sortByCategory) {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        QSettings().setValue("Videos/Sortby", "category");

        QFont font; font.setPointSize(18); ui->listWidget->setFont(font);
        ui->listWidget->setAlternatingRowColors(true);
        ui->listWidget->setViewMode(QListView::ListMode);
        ui->listWidget->setItemDelegate(new MediaWithIconDelegate(ui->listWidget));
        ui->listWidget->setWrapping(false);
        ui->listWidget->setGridSize(QSize());
        ui->listWidget->setFlow(QListView::TopToBottom);
    }

    ui->listWidget->clear();
    this->listVideos();
}

void VideosWindow::selectView()
{
    if (QSettings().value("Videos/Sortby").toString() == "category") {
        sortByCategory->setChecked(true);
        onSortingChanged(sortByCategory);
    } else {
        sortByDate->setChecked(true);
        onSortingChanged(sortByDate);
    }
}

#ifdef MAFW
void VideosWindow::listVideos()
{
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

#ifdef DEBUG
    qDebug("Source ready");
#endif
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllVideos(uint, int, uint, QString, GHashTable*, QString)), Qt::UniqueConnection);

    this->browseId = mafwTrackerSource->sourceBrowse("localtagfs::videos", false, NULL, sortByDate->isChecked() ? "-added,+title" : NULL,
                                                      MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                       MAFW_METADATA_KEY_DURATION,
                                                                       MAFW_METADATA_KEY_THUMBNAIL_URI,
                                                                       MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI,
                                                                       MAFW_METADATA_KEY_VIDEO_SOURCE),
                                                      0, MAFW_SOURCE_BROWSE_ALL);
}

void VideosWindow::browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString)
{
    if (this->browseId != browseId) return;

    if (index == 0) {
        int delta = remainingCount - ui->listWidget->count() + (sortByDate->isChecked() ? 1 : 3); // one for the current item, additional 2 for the labels
        if (delta > 0)
            for (int i = 0; i < delta; i++)
                ui->listWidget->addItem(new QListWidgetItem());
        else
            for (int i = delta; i < 0; i++)
                delete ui->listWidget->item(ui->listWidget->count()-1);

        if (sortByCategory->isChecked()) bufferList.clear();
    }

    if (metadata != NULL) {
        QString title;
        QString source;
        int duration;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown clip)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_VIDEO_SOURCE);
        source = v ? QString::fromUtf8(g_value_get_string (v)) : "";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : Duration::Unknown;

        QListWidgetItem *item = sortByCategory->isChecked() ? new QListWidgetItem(&bufferList) :
                                                              ui->listWidget->item(index);

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

        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleTitle, title);

        if (sortByCategory->isChecked()) {
            item->setText((source.startsWith("noki://") ? "1_" : "3_") + title);
            item->setData(UserRoleSongDuration, duration);
        }
        else { // sortByDate->isChecked()
            if (duration != Duration::Unknown) {
                QTime t(0, 0);
                t = t.addSecs(duration);
                item->setData(UserRoleValueText, t.toString("h:mm:ss"));
            } else
                item->setData(UserRoleValueText, "-:--:--");
        }
    }

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                   this, SLOT(browseAllVideos(uint, int, uint, QString, GHashTable*, QString)));

        if (sortByCategory->isChecked()) {
            QListWidgetItem *item;
            item = new QListWidgetItem(&bufferList);
            item->setText("0_");
            item->setData(UserRoleSongDuration, Duration::Blank);
            item->setData(UserRoleTitle, tr("Recorded by device camera"));
            item = new QListWidgetItem(&bufferList);
            item->setText("2_");
            item->setData(UserRoleSongDuration, Duration::Blank);
            item->setData(UserRoleTitle, tr("Films"));
            bufferList.sortItems();
            for (int i = 0; i < ui->listWidget->count(); i++) {
                ui->listWidget->item(i)->setData(UserRoleObjectID, bufferList.item(i)->data(UserRoleObjectID));
                ui->listWidget->item(i)->setData(UserRoleTitle, bufferList.item(i)->data(UserRoleTitle));
                ui->listWidget->item(i)->setData(UserRoleValueText, bufferList.item(i)->data(UserRoleValueText));
                ui->listWidget->item(i)->setData(UserRoleSongDuration, bufferList.item(i)->data(UserRoleSongDuration));
                ui->listWidget->item(i)->setIcon(bufferList.item(i)->icon());
            }
            bufferList.clear();
        }

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
        QTimer::singleShot(3000, this, SLOT(listVideos())); // some time for the thumbnailer to finish
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
