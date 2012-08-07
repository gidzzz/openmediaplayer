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
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif

    ThumbnailItemDelegate *delegate = new ThumbnailItemDelegate(ui->videoList);
    ui->videoList->setItemDelegate(delegate);

    ui->videoList->installEventFilter(this);
    ui->videoList->viewport()->installEventFilter(this);

    videoModel = new QStandardItemModel(this);
    videoProxyModel = new HeaderAwareProxyModel(this);
    videoProxyModel->setFilterRole(UserRoleTitle);
    videoProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    videoProxyModel->setSourceModel(videoModel);
    ui->videoList->setModel(videoProxyModel);

    QActionGroup *sortByActionGroup = new QActionGroup(this);
    sortByActionGroup->setExclusive(true);
    sortByDate = new QAction(tr("Date"), sortByActionGroup);
    sortByDate->setCheckable(true);
    sortByCategory = new QAction(tr("Category"), sortByActionGroup);
    sortByCategory->setCheckable(true);
    this->menuBar()->addActions(sortByActionGroup->actions());

    connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());

#ifdef MAFW
    if (mafwTrackerSource->isReady())
        selectView();
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
    connect(ui->videoList, SIGNAL(activated(QModelIndex)), this, SLOT(onVideoSelected(QModelIndex)));
    connect(ui->videoList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->videoList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));

    connect(ui->menubar, SIGNAL(triggered(QAction*)), this, SLOT(onSortingChanged(QAction*)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), videoProxyModel, SLOT(setFilterFixedString(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));

#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(metadataChanged(QString)), this, SLOT(onSourceMetadataChanged(QString)));
    connect(mafwrenderer, SIGNAL(metadataChanged(QString, QVariant)), this, SLOT(onMetadataChanged(QString,QVariant)));
#endif
}

void VideosWindow::onContextMenuRequested(const QPoint &pos)
{
    if (ui->videoList->currentIndex().data(UserRoleHeader).toBool()) return;

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), contextMenu), SIGNAL(activated()), contextMenu, SLOT(close()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void VideosWindow::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->videoList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
        videoProxyModel->removeRow(ui->videoList->currentIndex().row());
    }
#endif
    ui->videoList->clearSelection();
}

void VideosWindow::onShareClicked()
{
#ifdef MAFW
    mafwTrackerSource->getUri(ui->videoList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
    connect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));
#endif
}

#ifdef MAFW
void VideosWindow::onShareUriReceived(QString objectId, QString uri)
{
    disconnect(mafwTrackerSource, SIGNAL(signalGotUri(QString,QString)), this, SLOT(onShareUriReceived(QString,QString)));

    if (objectId != ui->videoList->currentIndex().data(UserRoleObjectID).toString()) return;

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

void VideosWindow::onVideoSelected(QModelIndex index)
{
    if (index.data(UserRoleHeader).toBool()) return;

    this->setEnabled(false);

#ifdef MAFW
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory);
#else
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
#endif
    window->showFullScreen();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();

    playlist->assignVideoPlaylist();
    playlist->clear();

    int selectedRow = videoProxyModel->mapToSource(index).row();
    int indexOffset = 0;
    int videoCount = 0;
    gchar** videoAddBuffer = new gchar*[videoModel->rowCount()+1];

    for (int i = 0; i < videoModel->rowCount(); i++)
        if (!videoModel->item(i)->data(UserRoleHeader).toBool())
            videoAddBuffer[videoCount++] = qstrdup(videoModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
        else if (i < selectedRow)
            ++indexOffset;
    videoAddBuffer[videoCount] = NULL;

    playlist->appendItems((const gchar**)videoAddBuffer);

    for (int i = 0; i < videoCount; i++)
        delete[] videoAddBuffer[i];
    delete[] videoAddBuffer;

    mafwrenderer->gotoIndex(selectedRow-indexOffset);
    QTimer::singleShot(500, window, SLOT(playVideo()));
}

void VideosWindow::onSortingChanged(QAction *action)
{
    if (action == sortByDate) {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        QSettings().setValue("Videos/Sortby", "date");

        QFont font; font.setPointSize(13); ui->videoList->setFont(font);
        ui->videoList->setAlternatingRowColors(false);
        ui->videoList->setViewMode(QListView::IconMode);
        ui->videoList->setItemDelegate(new ThumbnailItemDelegate(ui->videoList));
        ui->videoList->setWrapping(true);
        ui->videoList->setGridSize(QSize(155,215));
        ui->videoList->setFlow(QListView::LeftToRight);

    } else if (action == sortByCategory) {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        QSettings().setValue("Videos/Sortby", "category");

        QFont font; font.setPointSize(18); ui->videoList->setFont(font);
        ui->videoList->setAlternatingRowColors(true);
        ui->videoList->setViewMode(QListView::ListMode);
        ui->videoList->setItemDelegate(new MediaWithIconDelegate(ui->videoList));
        ui->videoList->setWrapping(false);
        ui->videoList->setGridSize(QSize());
        ui->videoList->setFlow(QListView::TopToBottom);
    }

    videoModel->clear();
    listVideos();
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

void VideosWindow::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void VideosWindow::onSearchTextChanged(QString text)
{
    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void VideosWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_Shift:
            onContextMenuRequested(QPoint(35,35));
            break;
    }
}

void VideosWindow::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Backspace:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            return;

        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            ui->videoList->setFocus();
            break;

        default:
            ui->videoList->clearSelection();
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

    browseId = mafwTrackerSource->sourceBrowse("localtagfs::videos", false, NULL, sortByDate->isChecked() ? "-added,+title" : "+title",
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
        recordingsBufferList.clear();
        filmsBufferList.clear();

        if (sortByDate->isChecked()) {
            int delta = remainingCount+1 - videoModel->rowCount();
            if (delta > 0)
                for (int i = 0; i < delta; i++)
                    videoModel->appendRow(new QStandardItem());
            else
                for (int i = delta; i < 0; i++)
                    videoModel->removeRow(videoModel->rowCount()-1);
        }
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

        QStandardItem *item = sortByCategory->isChecked() ? new QStandardItem() : videoModel->item(index);

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

        item->setData(objectId, UserRoleObjectID);
        item->setData(title, UserRoleTitle);

        if (sortByCategory->isChecked()) {
            item->setData(duration, UserRoleSongDuration);
            (source.startsWith("noki://") ? recordingsBufferList : filmsBufferList).append(item);
        }
        else { // sortByDate->isChecked()
            if (duration != Duration::Unknown) {
                QTime t(0, 0);
                t = t.addSecs(duration);
                item->setData(t.toString("h:mm:ss"), UserRoleValueText);
            } else
                item->setData("-:--:--", UserRoleValueText);
        }
    }

    if (remainingCount == 0) {
        disconnect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllVideos(uint,int,uint,QString,GHashTable*,QString)));

        if (sortByCategory->isChecked()) {
            bool drawHeaders = !recordingsBufferList.isEmpty() && !filmsBufferList.isEmpty();
            int delta = recordingsBufferList.size() + filmsBufferList.size() - videoModel->rowCount();
            if (drawHeaders) delta += 2;

            if (delta > 0)
                for (int i = 0; i < delta; i++)
                    videoModel->appendRow(new QStandardItem());
            else
                for (int i = delta; i < 0; i++)
                    videoModel->removeRow(videoModel->rowCount()-1);

            int i = 0;

            if (!recordingsBufferList.isEmpty()) {
                if (drawHeaders) {
                    videoModel->item(i)->setData(true, UserRoleHeader);
                    videoModel->item(i)->setData(tr("Recorded by device camera"), UserRoleTitle);
                    videoModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                    ++i;
                }

                while (!recordingsBufferList.isEmpty()) {
                    videoModel->item(i)->setData(false, UserRoleHeader);
                    videoModel->item(i)->setData(recordingsBufferList.first()->data(UserRoleTitle), UserRoleTitle);
                    videoModel->item(i)->setData(recordingsBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                    videoModel->item(i)->setData(recordingsBufferList.first()->data(UserRoleSongDuration), UserRoleSongDuration);
                    videoModel->item(i)->setIcon(recordingsBufferList.first()->icon());
                    delete recordingsBufferList.takeFirst();
                    ++i;
                }
            }

            if (!filmsBufferList.isEmpty()) {
                if (drawHeaders) {
                    videoModel->item(i)->setData(true, UserRoleHeader);
                    videoModel->item(i)->setData(tr("Films"), UserRoleTitle);
                    videoModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                    ++i;
                }

                while (!filmsBufferList.isEmpty()) {
                    videoModel->item(i)->setData(false, UserRoleHeader);
                    videoModel->item(i)->setData(filmsBufferList.first()->data(UserRoleTitle), UserRoleTitle);
                    videoModel->item(i)->setData(filmsBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                    videoModel->item(i)->setData(filmsBufferList.first()->data(UserRoleSongDuration), UserRoleSongDuration);
                    videoModel->item(i)->setIcon(filmsBufferList.first()->icon());
                    delete filmsBufferList.takeFirst();
                    ++i;
                }
            }
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

void VideosWindow::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
}

bool VideosWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Resize && obj == ui->videoList)
        ui->videoList->setFlow(ui->videoList->flow());

    else if (e->type() == QEvent::MouseButtonPress && obj == ui->videoList->viewport()
    && static_cast<QMouseEvent*>(e)->y() > ui->videoList->viewport()->height() - 25
    && ui->searchWidget->isHidden()) {
        ui->indicator->inhibit();
        ui->searchWidget->show();
    }

    return false;
}

void VideosWindow::onChildClosed()
{
    ui->indicator->restore();
    ui->videoList->clearSelection();
    this->setEnabled(true);
}
