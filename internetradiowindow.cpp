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
    ui->listWidget->setItemDelegate(new SongListItemDelegate(ui->listWidget));
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif

    this->connectSignals();

    ui->listWidget->viewport()->installEventFilter(this);

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());

#ifdef MAFW
    if (mafwRadioSource->isReady())
        this->listStations();
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
    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(onAddClicked()));
    connect(ui->listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onStationSelected(QListWidgetItem*)));
    connect(ui->listWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
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

void InternetRadioWindow::onStationSelected(QListWidgetItem* item)
{
    this->setEnabled(false);

#ifdef MAFW
    playlist->assignRadioPlaylist();

    playlist->clear();

    int stationCount = ui->listWidget->count();
    gchar** songAddBuffer = new gchar*[stationCount+1];

    for (int i = 0; i < stationCount; i++)
        songAddBuffer[i] = qstrdup(ui->listWidget->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[stationCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < stationCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    playlist->getSize(); // explained in musicwindow.cpp
    mafwrenderer->gotoIndex(ui->listWidget->row(item));

    window = new RadioNowPlayingWindow(this, mafwFactory);
#else
    window = new RadioNowPlayingWindow(this);
#endif
    window->show();
#ifdef MAFW
    window->play();
#endif

    connect(window, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
    ui->indicator->inhibit();
}

void InternetRadioWindow::onContextMenuRequested(const QPoint &point)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->addAction(tr("Edit"), this, SLOT(onEditClicked()));
    contextMenu->addAction(tr("Delete"), this, SLOT(onDeleteClicked()));
    contextMenu->exec(point);
}

void InternetRadioWindow::onEditClicked()
{
#ifdef MAFW
    QListWidgetItem *item = ui->listWidget->currentItem();

    if (BookmarkDialog(this, mafwFactory,
                       item->text(),
                       item->data(UserRoleValueText).toString(),
                       item->data(UserRoleObjectID).toString())
        .exec() == QDialog::Accepted)
        listStations();
#endif
}

void InternetRadioWindow::onDeleteClicked()
{
#ifdef MAFW
    QMessageBox confirmDelete(QMessageBox::NoIcon,
                              " ",
                              tr("Delete selected item from device?"),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                              this);
    confirmDelete.button(QMessageBox::Yes)->setText(tr("Yes"));
    confirmDelete.button(QMessageBox::No)->setText(tr("No"));
    confirmDelete.exec();
    if (confirmDelete.result() == QMessageBox::Yes) {
        mafwRadioSource->destroyObject(ui->listWidget->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->listWidget->currentItem();
    }
#endif
    ui->listWidget->clearSelection();
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

    qDebug() << "connecting InternetRadioWindow to signalSourceBrowseResult";
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
        int delta = remainingCount - ui->listWidget->count() + 1;
        if (delta > 0)
            for (int i = 0; i < delta; i++)
                ui->listWidget->addItem(new QListWidgetItem());
        else
            for (int i = delta; i < 0; i++)
                delete ui->listWidget->item(ui->listWidget->count()-1);
    }

    if (metadata != NULL) {
        QString title;
        QString URI;
        QString mime;
        GValue *v;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME);
        mime = QString::fromUtf8(g_value_get_string (v));

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown station)");
        if (mime.contains("video")) title = "[VIDEO] " + title;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        URI = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown)");

        QListWidgetItem *item = ui->listWidget->item(index);

        item->setText(title);
        item->setData(UserRoleValueText, URI);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, Duration::Blank);
    }

    if (remainingCount == 0) {
        qDebug() << "disconnecting InternetRadioWindow from signalSourceBrowseResult";
        disconnect(mafwRadioSource, SIGNAL(signalSourceBrowseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(browseAllStations(uint,int,uint,QString,GHashTable*,QString)));
        if (!ui->searchEdit->text().isEmpty())
            this->onSearchTextChanged(ui->searchEdit->text());
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
    }
}

void InternetRadioWindow::onContainerChanged()
{
    this->listStations();
}
#endif

void InternetRadioWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
        this->close();
}

bool InternetRadioWindow::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress
    && static_cast<QMouseEvent*>(e)->y() > ui->listWidget->viewport()->height() - 25
    && ui->searchWidget->isHidden()) {
        ui->indicator->inhibit();
        ui->searchWidget->show();
    }
    return false;
}

void InternetRadioWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down)
        ui->listWidget->setFocus();
    else {
        ui->listWidget->clearSelection();
        if (ui->searchWidget->isHidden()) {
            ui->indicator->inhibit();
            ui->searchWidget->show();
        }
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
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
    for (int i = 0; i < ui->listWidget->count(); i++) {
        if (ui->listWidget->item(i)->text().contains(text, Qt::CaseInsensitive)
        || ui->listWidget->item(i)->data(UserRoleValueText).toString().contains(text, Qt::CaseInsensitive))
            ui->listWidget->item(i)->setHidden(false);
        else
            ui->listWidget->item(i)->setHidden(true);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        ui->indicator->restore();
    }
}

void InternetRadioWindow::orientationChanged(int w, int h)
{
    ui->listWidget->scroll(1, 1);
    ui->indicator->setGeometry(w-122, h-(70+55), ui->indicator->width(), ui->indicator->height());
    ui->indicator->raise();
}

void InternetRadioWindow::onChildClosed()
{
    ui->indicator->restore();
    ui->listWidget->clearSelection();
    this->setEnabled(true);
}

void InternetRadioWindow::focusInEvent(QFocusEvent *)
{
    ui->indicator->triggerAnimation();
}

void InternetRadioWindow::focusOutEvent(QFocusEvent *)
{
    ui->indicator->stopAnimation();
}
