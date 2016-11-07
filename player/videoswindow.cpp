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

VideosWindow::VideosWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry) :
    BrowserWindow(parent, mafwRegistry),
    mafwRegistry(mafwRegistry),
    mafwRenderer(mafwRegistry->renderer()),
    mafwTrackerSource(mafwRegistry->source(MafwRegistryAdapter::Tracker)),
    playlist(mafwRegistry->playlist())
{
    ui->objectList->setIconSize(QSize(64, 64));
    ui->objectList->setDragDropMode(QAbstractItemView::NoDragDrop);
    ui->objectList->setMovement(QListView::Static);

    ui->objectList->setItemDelegate(new ThumbnailItemDelegate(ui->objectList));

    objectProxyModel->setDynamicSortFilter(true);
    objectProxyModel->setFilterRole(UserRoleTitle);

    QActionGroup *sortByActionGroup = new QActionGroup(this);
    sortByDate = new QAction(tr("Date"), sortByActionGroup);
    sortByDate->setCheckable(true);
    sortByCategory = new QAction(tr("Category"), sortByActionGroup);
    sortByCategory->setCheckable(true);
    ui->windowMenu->addActions(sortByActionGroup->actions());

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->windowMenu, SIGNAL(triggered(QAction*)), this, SLOT(onSortingChanged(QAction*)));

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onVideoSelected(QModelIndex)));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    if (mafwTrackerSource->isReady()) {
        onSourceReady();
    } else {
        connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onSourceReady()));
    }
}

void VideosWindow::onSourceReady()
{
    disconnect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onSourceReady()));
    connect(mafwTrackerSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged(QString)));
    connect(mafwTrackerSource, SIGNAL(metadataChanged(QString)), this, SLOT(onMetadataChanged(QString)));

    selectView();
}

void VideosWindow::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().data(UserRoleHeader).toBool()) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Share"), this, SLOT(onShareClicked()));
    contextMenu->addAction(tr("Details"), this, SLOT(onDetailsClicked()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void VideosWindow::onDeleteClicked()
{
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwTrackerSource->destroyObject(ui->objectList->currentIndex().data(UserRoleObjectID).toString());
        objectProxyModel->removeRow(ui->objectList->currentIndex().row());
    }
    ui->objectList->clearSelection();
}

void VideosWindow::onShareClicked()
{
    (new ShareDialog(this, mafwTrackerSource, ui->objectList->currentIndex().data(UserRoleObjectID).toString()))->show();
}

void VideosWindow::onDetailsClicked()
{
    (new MetadataDialog(this, mafwTrackerSource, ui->objectList->currentIndex().data(UserRoleObjectID).toString()))->show();
}

void VideosWindow::onVideoSelected(QModelIndex index)
{
    if (index.data(UserRoleHeader).toBool()) return;

    this->setEnabled(false);

    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwRegistry);
    window->showFullScreen();

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();

    playlist->assignVideoPlaylist();
    playlist->clear();

    int selectedRow;
    int indexOffset = 0;
    int videoCount = 0;
    gchar** videoAddBuffer = new gchar*[objectModel->rowCount()+1];

    bool filter = QSettings().value("main/playlistFilter", false).toBool();

    if (filter) {
        selectedRow = index.row();
        for (int i = 0; i < objectProxyModel->rowCount(); i++)
            if (!objectProxyModel->index(i,0).data(UserRoleHeader).toBool())
                videoAddBuffer[videoCount++] = qstrdup(objectProxyModel->index(i,0).data(UserRoleObjectID).toString().toUtf8());
            else if (i < selectedRow)
                ++indexOffset;
    } else {
        selectedRow = objectProxyModel->mapToSource(index).row();
        for (int i = 0; i < objectModel->rowCount(); i++)
            if (!objectModel->item(i)->data(UserRoleHeader).toBool())
                videoAddBuffer[videoCount++] = qstrdup(objectModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
            else if (i < selectedRow)
                ++indexOffset;
    }

    videoAddBuffer[videoCount] = NULL;

    playlist->appendItems((const gchar**) videoAddBuffer);

    for (int i = 0; i < videoCount; i++)
        delete[] videoAddBuffer[i];
    delete[] videoAddBuffer;

    mafwRenderer->gotoIndex(selectedRow-indexOffset);
    window->play();
}

void VideosWindow::onSortingChanged(QAction *action)
{
    if (action == sortByDate) {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        QSettings().setValue("Videos/Sortby", "date");

        QFont font; font.setPointSize(13); ui->objectList->setFont(font);
        ui->objectList->setAlternatingRowColors(false);
        ui->objectList->setViewMode(QListView::IconMode);
        ui->objectList->itemDelegate()->deleteLater();
        ui->objectList->setItemDelegate(new ThumbnailItemDelegate(ui->objectList));
        ui->objectList->setWrapping(true);
        ui->objectList->setFlow(QListView::LeftToRight);
        // Set grid size depending on the orientation
        this->orientationInit();

    } else if (action == sortByCategory) {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        QSettings().setValue("Videos/Sortby", "category");

        QFont font; font.setPointSize(18); ui->objectList->setFont(font);
        ui->objectList->setAlternatingRowColors(true);
        ui->objectList->setViewMode(QListView::ListMode);
        ui->objectList->itemDelegate()->deleteLater();
        ui->objectList->setItemDelegate(new MediaWithIconDelegate(ui->objectList));
        ui->objectList->setWrapping(false);
        ui->objectList->setFlow(QListView::TopToBottom);
        ui->objectList->setGridSize(QSize());
    }

    objectModel->clear();
    listVideos();
}

void VideosWindow::selectView()
{
    if (QSettings().value("Videos/Sortby", "date").toString() == "category") {
        sortByCategory->setChecked(true);
        onSortingChanged(sortByCategory);
    } else {
        sortByDate->setChecked(true);
        onSortingChanged(sortByDate);
    }
}

void VideosWindow::listVideos()
{
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);

#ifdef DEBUG
    qDebug("Source ready");
#endif

    connect(mafwTrackerSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllVideos(uint,int,uint,QString,GHashTable*)), Qt::UniqueConnection);

    browseId = mafwTrackerSource->browse("localtagfs::videos", false, NULL, sortByDate->isChecked() ? "-added,+title" : "+title",
                                         MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                          MAFW_METADATA_KEY_DURATION,
                                                          MAFW_METADATA_KEY_THUMBNAIL_URI,
                                                          MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI,
                                                          MAFW_METADATA_KEY_VIDEO_SOURCE),
                                         0, MAFW_SOURCE_BROWSE_ALL);
}

void VideosWindow::browseAllVideos(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata)
{
    if (this->browseId != browseId) return;

    if (index == 0) {
        recordingsBufferList.clear();
        filmsBufferList.clear();

        if (sortByDate->isChecked()) {
            int delta = remainingCount+1 - objectModel->rowCount();
            if (delta > 0)
                for (int i = 0; i < delta; i++)
                    objectModel->appendRow(new QStandardItem());
            else
                for (int i = delta; i < 0; i++)
                    objectModel->removeRow(objectModel->rowCount()-1);
        }
    }

    if (metadata != NULL) {
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        QString title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown clip)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_VIDEO_SOURCE);
        QString source = v ? QString::fromUtf8(g_value_get_string (v)) : QString();

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_DURATION);
        int duration = v ? g_value_get_int (v) : Duration::Unknown;

        QStandardItem *item = sortByCategory->isChecked() ? new QStandardItem() : objectModel->item(index);

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_PAUSED_THUMBNAIL_URI);
        if (v != NULL) {
            const gchar* filename = g_value_get_string(v); // the uri is really a filename
            if (filename != NULL)
                item->setIcon(QIcon(QString::fromUtf8(filename)));
        } else {
            v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_THUMBNAIL_URI);
            if (v != NULL) {
                const gchar* file_uri = g_value_get_string(v); // here uri is a uri
                gchar* filename;
                if (file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL)
                    item->setIcon(QIcon(QString::fromUtf8(filename)));
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
        disconnect(mafwTrackerSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllVideos(uint,int,uint,QString,GHashTable*)));

        if (sortByCategory->isChecked()) {
            bool drawHeaders = !recordingsBufferList.isEmpty() && !filmsBufferList.isEmpty();
            int delta = recordingsBufferList.size() + filmsBufferList.size() - objectModel->rowCount();
            if (drawHeaders) delta += 2;

            if (delta > 0)
                for (int i = 0; i < delta; i++)
                    objectModel->appendRow(new QStandardItem());
            else
                for (int i = delta; i < 0; i++)
                    objectModel->removeRow(objectModel->rowCount()-1);

            int i = 0;

            if (!recordingsBufferList.isEmpty()) {
                if (drawHeaders) {
                    objectModel->item(i)->setData(true, UserRoleHeader);
                    objectModel->item(i)->setData(tr("Recorded by device camera"), UserRoleTitle);
                    objectModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                    ++i;
                }

                while (!recordingsBufferList.isEmpty()) {
                    objectModel->item(i)->setData(false, UserRoleHeader);
                    objectModel->item(i)->setData(recordingsBufferList.first()->data(UserRoleTitle), UserRoleTitle);
                    objectModel->item(i)->setData(recordingsBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                    objectModel->item(i)->setData(recordingsBufferList.first()->data(UserRoleSongDuration), UserRoleSongDuration);
                    objectModel->item(i)->setIcon(recordingsBufferList.first()->icon());
                    delete recordingsBufferList.takeFirst();
                    ++i;
                }
            }

            if (!filmsBufferList.isEmpty()) {
                if (drawHeaders) {
                    objectModel->item(i)->setData(true, UserRoleHeader);
                    objectModel->item(i)->setData(tr("Films"), UserRoleTitle);
                    objectModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                    ++i;
                }

                while (!filmsBufferList.isEmpty()) {
                    objectModel->item(i)->setData(false, UserRoleHeader);
                    objectModel->item(i)->setData(filmsBufferList.first()->data(UserRoleTitle), UserRoleTitle);
                    objectModel->item(i)->setData(filmsBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                    objectModel->item(i)->setData(filmsBufferList.first()->data(UserRoleSongDuration), UserRoleSongDuration);
                    objectModel->item(i)->setIcon(filmsBufferList.first()->icon());
                    delete filmsBufferList.takeFirst();
                    ++i;
                }
            }
        }

        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    }
}

void VideosWindow::onMetadataChanged(QString objectId)
{
    if (objectId.startsWith("localtagfs::videos"))
        this->listVideos();
}

void VideosWindow::onContainerChanged(QString objectId)
{
    if (objectId == "localtagfs::videos" || objectId.startsWith("localtagfs::videos"))
        QTimer::singleShot(3000, this, SLOT(listVideos())); // some time for the thumbnailer to finish
}
