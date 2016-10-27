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

#include "internetradiowindow.h"

InternetRadioWindow::InternetRadioWindow(QWidget *parent, MafwRegistryAdapter *mafwRegistry) :
    BrowserWindow(parent, mafwRegistry),
    mafwRegistry(mafwRegistry),
    mafwRenderer(mafwRegistry->renderer()),
    mafwRadioSource(mafwRegistry->source(MafwRegistryAdapter::Radio)),
    playlist(mafwRegistry->playlist())
{
    this->setWindowTitle(tr("Internet radio stations"));

    ui->objectList->setItemDelegate(new SongListItemDelegate(ui->objectList));

    objectProxyModel->setFilterRole(UserRoleFilterString);

    ui->windowMenu->addAction(tr("Add radio bookmark"), this, SLOT(onAddClicked()));
    ui->windowMenu->addAction(tr("FM transmitter"    ), this, SLOT(showFMTXDialog()));

    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));

    connect(ui->objectList, SIGNAL(activated(QModelIndex)), this, SLOT(onStationSelected(QModelIndex)));
    connect(ui->objectList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(mafwRadioSource, SIGNAL(containerChanged(QString)), this, SLOT(listStations()));
    if (mafwRadioSource->isReady())
        listStations();
}

void InternetRadioWindow::showFMTXDialog()
{
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
}

void InternetRadioWindow::onStationSelected(QModelIndex index)
{
    if (index.data(UserRoleHeader).toBool()) return;

    this->setEnabled(false);

    QString type = index.data(UserRoleMIME).toString().startsWith("audio") ? "audio" : "video";

    if (type == "audio")
        playlist->assignRadioPlaylist();
    else // type == "video"
        playlist->assignVideoPlaylist();

    playlist->clear();

    int stationCount = objectModel->rowCount();
    gchar** songAddBuffer = new gchar*[stationCount+1];

    int selectedRow = objectProxyModel->mapToSource(index).row();
    int sameTypeIndex = 0;
    int sameTypeCount = 0;
    for (int i = 0; i < selectedRow; i++)
        if (objectModel->item(i)->data(UserRoleMIME).toString().startsWith(type))
            ++sameTypeIndex;
    for (int i = 0; i < stationCount; i++)
        if (objectModel->item(i)->data(UserRoleMIME).toString().startsWith(type))
            songAddBuffer[sameTypeCount++] = qstrdup(objectModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[sameTypeCount] = NULL;

    playlist->appendItems((const gchar**) songAddBuffer);

    for (int i = 0; i < sameTypeCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    mafwRenderer->gotoIndex(sameTypeIndex);

    if (type == "audio") {
        RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this, mafwRegistry);
        window->show();
        window->play();
        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    } else { // type == "video"
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwRegistry);
        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
        window->showFullScreen();
        window->play();
    }

    ui->indicator->inhibit();
}

void InternetRadioWindow::onContextMenuRequested(const QPoint &pos)
{
    if (ui->objectList->currentIndex().data(UserRoleHeader).toBool()) return;

    QMenu *contextMenu = new KbMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Edit"), this, SLOT(onEditClicked()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->addAction(tr("Details"), this, SLOT(onDetailsClicked()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void InternetRadioWindow::onEditClicked()
{
    QModelIndex index = ui->objectList->currentIndex();

    if (BookmarkDialog(this, mafwRegistry,
                       index.data(UserRoleMIME).toString().startsWith("audio") ? Media::Audio : Media::Video,
                       index.data(UserRoleValueText).toString(), index.data(Qt::DisplayRole).toString(),
                       index.data(UserRoleObjectID).toString())
        .exec() == QDialog::Accepted)
    {
        listStations();
    }
}

void InternetRadioWindow::onDeleteClicked()
{
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwRadioSource->destroyObject(ui->objectList->currentIndex().data(UserRoleObjectID).toString());
        objectProxyModel->removeRow(ui->objectList->currentIndex().row());
    }
    ui->objectList->clearSelection();
}

void InternetRadioWindow::onDetailsClicked()
{
    (new MetadataDialog(this, mafwRadioSource, ui->objectList->currentIndex().data(UserRoleObjectID).toString()))->show();
}

void InternetRadioWindow::onAddClicked()
{
    BookmarkDialog(this, mafwRegistry).exec();
}

void InternetRadioWindow::listStations()
{
#ifdef DEBUG
    qDebug("Source ready");
#endif

    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);

    connect(mafwRadioSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllStations(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseId = mafwRadioSource->browse("iradiosource::", false, NULL, "+title",
                                       MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                        MAFW_METADATA_KEY_URI,
                                                        MAFW_METADATA_KEY_MIME),
                                       0, MAFW_SOURCE_BROWSE_ALL);
}

void InternetRadioWindow::browseAllStations(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString)
{
    if (this->browseId != browseId) return;

    if (index == 0) {
        audioBufferList.clear();
        videoBufferList.clear();
    }

    if (metadata != NULL) {
        QString title;
        QString mime;
        QString uri;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown station)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME);
        mime = QString::fromUtf8(g_value_get_string (v));

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        uri = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown)");

        QStandardItem *item = new QStandardItem();
        item->setText(title);
        item->setData(mime, UserRoleMIME);
        item->setData(uri, UserRoleValueText);
        item->setData(objectId, UserRoleObjectID);

        (mime.startsWith("audio") ? audioBufferList : videoBufferList).append(item);
    }

    if (remainingCount == 0) {
        disconnect(mafwRadioSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllStations(uint,int,uint,QString,GHashTable*,QString)));

        bool drawHeaders = !audioBufferList.isEmpty() && !videoBufferList.isEmpty();
        int delta = audioBufferList.size() + videoBufferList.size() - objectModel->rowCount();
        if (drawHeaders) delta += 2;
        if (delta > 0)
            for (int i = 0; i < delta; i++)
                objectModel->appendRow(new QStandardItem());
        else
            for (int i = delta; i < 0; i++)
                objectModel->removeRow(objectModel->rowCount()-1);

        int i = 0;

        if (!audioBufferList.isEmpty()) {
            if (drawHeaders) {
                objectModel->item(i)->setData(true, UserRoleHeader);
                objectModel->item(i)->setText(tr("Audio bookmarks"));
                objectModel->item(i)->setData(QVariant(), UserRoleMIME);
                ++i;
            }

            while (!audioBufferList.isEmpty()) {
                objectModel->item(i)->setData(false, UserRoleHeader);
                objectModel->item(i)->setText(audioBufferList.first()->text());
                objectModel->item(i)->setData(audioBufferList.first()->data(UserRoleValueText), UserRoleValueText);
                objectModel->item(i)->setData(audioBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                objectModel->item(i)->setData(audioBufferList.first()->data(UserRoleMIME), UserRoleMIME);
                objectModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                objectModel->item(i)->setData(QString(audioBufferList.first()->text() % QChar(31) %
                                                      audioBufferList.first()->data(UserRoleValueText).toString()),
                                              UserRoleFilterString);
                delete audioBufferList.takeFirst();
                ++i;
            }
        }

        if (!videoBufferList.isEmpty()) {
            if (drawHeaders) {
                objectModel->item(i)->setData(true, UserRoleHeader);
                objectModel->item(i)->setText(tr("Video bookmarks"));
                objectModel->item(i)->setData(QVariant(), UserRoleMIME);
                ++i;
            }

            while (!videoBufferList.isEmpty()) {
                objectModel->item(i)->setData(false, UserRoleHeader);
                objectModel->item(i)->setText(videoBufferList.first()->text());
                objectModel->item(i)->setData(videoBufferList.first()->data(UserRoleValueText), UserRoleValueText);
                objectModel->item(i)->setData(videoBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                objectModel->item(i)->setData(videoBufferList.first()->data(UserRoleMIME), UserRoleMIME);
                objectModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                objectModel->item(i)->setData(QString(videoBufferList.first()->text() % QChar(31) %
                                                      videoBufferList.first()->data(UserRoleValueText).toString()),
                                              UserRoleFilterString);
                delete videoBufferList.takeFirst();
                ++i;
            }
        }

        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
    }
}
