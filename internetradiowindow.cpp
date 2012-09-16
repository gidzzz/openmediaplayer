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

InternetRadioWindow::InternetRadioWindow(QWidget *parent, MafwAdapterFactory *factory) :
    QMainWindow(parent),
    ui(new Ui::InternetRadioWindow)
#ifdef MAFW
    ,mafwFactory(factory),
    mafwrenderer(factory->getRenderer()),
    mafwRadioSource(factory->getRadioSource()),
    playlist(factory->getPlaylistAdapter())
#endif
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
    ui->stationList->setItemDelegate(new SongListItemDelegate(ui->stationList));

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif

    ui->stationList->viewport()->installEventFilter(this);

    stationModel = new QStandardItemModel(this);
    stationProxyModel = new HeaderAwareProxyModel(this);
    stationProxyModel->setFilterRole(UserRoleFilterString);
    stationProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    stationProxyModel->setSourceModel(stationModel);
    ui->stationList->setModel(stationProxyModel);

    connectSignals();

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());

#ifdef MAFW
    if (mafwRadioSource->isReady())
        listStations();
    else
        connect(mafwRadioSource, SIGNAL(sourceReady()), this, SLOT(listStations()));
#endif
}

InternetRadioWindow::~InternetRadioWindow()
{
    delete ui;
}

void InternetRadioWindow::connectSignals()
{
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(onContextMenuRequested()));
    connect(new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(showWindowMenu()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), ui->windowMenu), SIGNAL(activated()), ui->windowMenu, SLOT(close()));

    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(onAddClicked()));

    connect(ui->stationList, SIGNAL(activated(QModelIndex)), this, SLOT(onStationSelected(QModelIndex)));
    connect(ui->stationList->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->stationList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));

    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), stationProxyModel, SLOT(setFilterFixedString(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), this, SLOT(onSearchHideButtonClicked()));

#ifdef MAFW
    connect(mafwRadioSource, SIGNAL(containerChanged(QString)), this, SLOT(onContainerChanged()));
#endif
}

void InternetRadioWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
#endif
}

void InternetRadioWindow::onStationSelected(QModelIndex index)
{
    if (index.data(UserRoleHeader).toBool()) return;

    this->setEnabled(false);

#ifdef MAFW
    QString type = index.data(UserRoleMIME).toString().startsWith("audio") ? "audio" : "video";

    if (type == "audio")
        playlist->assignRadioPlaylist();
    else // type == "video"
        playlist->assignVideoPlaylist();

    playlist->clear();

    int stationCount = stationModel->rowCount();
    gchar** songAddBuffer = new gchar*[stationCount+1];

    int selectedRow = stationProxyModel->mapToSource(index).row();
    int sameTypeIndex = 0;
    int sameTypeCount = 0;
    for (int i = 0; i < selectedRow; i++)
        if (stationModel->item(i)->data(UserRoleMIME).toString().startsWith(type))
            ++sameTypeIndex;
    for (int i = 0; i < stationCount; i++)
        if (stationModel->item(i)->data(UserRoleMIME).toString().startsWith(type))
            songAddBuffer[sameTypeCount++] = qstrdup(stationModel->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[sameTypeCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < sameTypeCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    mafwrenderer->gotoIndex(sameTypeIndex);

    if (type == "audio") {
        RadioNowPlayingWindow *window = new RadioNowPlayingWindow(this, mafwFactory);
        window->show();
        window->play();
        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    } else { // type == "video"
        VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this, mafwFactory);
        window->showFullScreen();
        QTimer::singleShot(500, mafwrenderer, SLOT(play()));
        connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    }
#else
    window = new RadioNowPlayingWindow(this);
    window->show();
    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
#endif

    ui->indicator->inhibit();
}

void InternetRadioWindow::onContextMenuRequested(const QPoint &pos)
{
    if (ui->stationList->currentIndex().data(UserRoleHeader).toBool()) return;

    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Edit"), this, SLOT(onEditClicked()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_Backspace), contextMenu), SIGNAL(activated()), contextMenu, SLOT(close()));
    contextMenu->exec(this->mapToGlobal(pos));
}

void InternetRadioWindow::showWindowMenu()
{
    ui->windowMenu->adjustSize();
    int x = (this->width() - ui->windowMenu->width()) / 2;
    ui->windowMenu->exec(this->mapToGlobal(QPoint(x,-35)));
}

void InternetRadioWindow::onEditClicked()
{
#ifdef MAFW
    QModelIndex index = ui->stationList->currentIndex();

    if (BookmarkDialog(this, mafwFactory,
                       index.data(UserRoleMIME).toString().startsWith("audio") ? Media::Audio : Media::Video,
                       index.data(UserRoleValueText).toString(), index.data(Qt::DisplayRole).toString(),
                       index.data(UserRoleObjectID).toString())
        .exec() == QDialog::Accepted)
    {
        listStations();
    }
#endif
}

void InternetRadioWindow::onDeleteClicked()
{
#ifdef MAFW
    if (ConfirmDialog(ConfirmDialog::Delete, this).exec() == QMessageBox::Yes) {
        mafwRadioSource->destroyObject(ui->stationList->currentIndex().data(UserRoleObjectID).toString().toUtf8());
        stationProxyModel->removeRow(ui->stationList->currentIndex().row());
    }
#endif
    ui->stationList->clearSelection();
}

void InternetRadioWindow::onAddClicked()
{
#ifdef MAFW
    BookmarkDialog(this, mafwFactory).exec();
#endif
}

#ifdef MAFW
void InternetRadioWindow::listStations()
{
#ifdef DEBUG
    qDebug("Source ready");
#endif

#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
#endif

    connect(mafwRadioSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(browseAllStations(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    browseId = mafwRadioSource->sourceBrowse("iradiosource::", false, NULL, "+title",
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
        disconnect(mafwRadioSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllStations(uint,int,uint,QString,GHashTable*,QString)));

        bool drawHeaders = !audioBufferList.isEmpty() && !videoBufferList.isEmpty();
        int delta = audioBufferList.size() + videoBufferList.size() - stationModel->rowCount();
        if (drawHeaders) delta += 2;
        if (delta > 0)
            for (int i = 0; i < delta; i++)
                stationModel->appendRow(new QStandardItem());
        else
            for (int i = delta; i < 0; i++)
                stationModel->removeRow(stationModel->rowCount()-1);

        int i = 0;

        if (!audioBufferList.isEmpty()) {
            if (drawHeaders) {
                stationModel->item(i)->setData(true, UserRoleHeader);
                stationModel->item(i)->setText(tr("Audio bookmarks"));
                stationModel->item(i)->setData(QVariant(), UserRoleMIME);
                ++i;
            }

            while (!audioBufferList.isEmpty()) {
                stationModel->item(i)->setData(false, UserRoleHeader);
                stationModel->item(i)->setText(audioBufferList.first()->text());
                stationModel->item(i)->setData(audioBufferList.first()->data(UserRoleValueText), UserRoleValueText);
                stationModel->item(i)->setData(audioBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                stationModel->item(i)->setData(audioBufferList.first()->data(UserRoleMIME), UserRoleMIME);
                stationModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                stationModel->item(i)->setData(QString(audioBufferList.first()->text() % QChar(31) %
                                                       audioBufferList.first()->data(UserRoleValueText).toString()),
                                               UserRoleFilterString);
                delete audioBufferList.takeFirst();
                ++i;
            }
        }

        if (!videoBufferList.isEmpty()) {
            if (drawHeaders) {
                stationModel->item(i)->setData(true, UserRoleHeader);
                stationModel->item(i)->setText(tr("Video bookmarks"));
                stationModel->item(i)->setData(QVariant(), UserRoleMIME);
                ++i;
            }

            while (!videoBufferList.isEmpty()) {
                stationModel->item(i)->setData(false, UserRoleHeader);
                stationModel->item(i)->setText(videoBufferList.first()->text());
                stationModel->item(i)->setData(videoBufferList.first()->data(UserRoleValueText), UserRoleValueText);
                stationModel->item(i)->setData(videoBufferList.first()->data(UserRoleObjectID), UserRoleObjectID);
                stationModel->item(i)->setData(videoBufferList.first()->data(UserRoleMIME), UserRoleMIME);
                stationModel->item(i)->setData(Duration::Blank, UserRoleSongDuration);
                stationModel->item(i)->setData(QString(videoBufferList.first()->text() % QChar(31) %
                                                       videoBufferList.first()->data(UserRoleValueText).toString()),
                                               UserRoleFilterString);
                delete videoBufferList.takeFirst();
                ++i;
            }
        }

#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void InternetRadioWindow::onContainerChanged()
{
    listStations();
}
#endif

void InternetRadioWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Space:
        case Qt::Key_Control:
        case Qt::Key_Shift:
            break;

        case Qt::Key_Backspace:
            this->close();
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->stationList->setFocus();
            break;

        default:
            ui->stationList->clearSelection();
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

void InternetRadioWindow::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            ui->stationList->setFocus();
    }
}

bool InternetRadioWindow::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress
    && static_cast<QMouseEvent*>(e)->y() > ui->stationList->viewport()->height() - 25
    && ui->searchWidget->isHidden()) {
        ui->indicator->inhibit();
        ui->searchWidget->show();
    }
    return false;
}

void InternetRadioWindow::onSearchHideButtonClicked()
{
    if (ui->searchEdit->text().isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    } else
        ui->searchEdit->clear();
}

void InternetRadioWindow::onSearchTextChanged(QString text)
{
    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void InternetRadioWindow::orientationChanged(int w, int h)
{
    ui->indicator->setGeometry(w-(112+8), h-(70+56), 112, 70);
    ui->indicator->raise();
}

void InternetRadioWindow::onChildClosed()
{
    ui->indicator->restore();
    ui->stationList->clearSelection();
    this->setEnabled(true);
}
