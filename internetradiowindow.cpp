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
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
#ifdef MAFW
    ui->indicator->setFactory(mafwFactory);
#endif
    SongListItemDelegate *delegate = new SongListItemDelegate(ui->listWidget);
    ui->listWidget->setItemDelegate(delegate);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    this->connectSignals();
    this->orientationChanged();
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
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(showBookmarkDialog()));
    connect(ui->listWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onStationSelected()));
    connect(ui->listWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->indicator, SLOT(poke()));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenuRequested(QPoint)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
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

void InternetRadioWindow::onStationSelected()
{
    this->setEnabled(false);

#ifdef MAFW
    playlist->assignRadioPlaylist();

    playlist->clear();

    int songCount = ui->listWidget->count();
    gchar** songAddBuffer = new gchar*[songCount+1];

    for (int i = 0; i < songCount; i++)
        songAddBuffer[i] = qstrdup(ui->listWidget->item(i)->data(UserRoleObjectID).toString().toUtf8());
    songAddBuffer[songCount] = NULL;

    playlist->appendItems((const gchar**)songAddBuffer);

    for (int i = 0; i < songCount; i++)
        delete[] songAddBuffer[i];
    delete[] songAddBuffer;

    playlist->getSize(); // explained in musicwindow.cpp
    mafwrenderer->gotoIndex(ui->listWidget->currentRow());

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
    QListWidgetItem *item = ui->listWidget->currentItem();
    bookmarkObjectId = item->data(UserRoleObjectID).toString();
    showBookmarkDialog(item->data(UserRoleSongTitle).toString(),
                       item->data(UserRoleValueText).toString());
}

void InternetRadioWindow::onDeleteClicked()
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
        mafwRadioSource->destroyObject(ui->listWidget->currentItem()->data(UserRoleObjectID).toString().toUtf8());
        delete ui->listWidget->currentItem();
    }
#endif
    ui->listWidget->clearSelection();
}

void InternetRadioWindow::showBookmarkDialog(QString name, QString address)
{
    bookmarkDialog = new QDialog(this);

    if (name.isEmpty()) {
        bookmarkObjectId = "";
        bookmarkDialog->setWindowTitle(tr("Add radio bookmark"));
    } else
        bookmarkDialog->setWindowTitle(tr("Edit radio bookmark"));

    // Labels
    nameLabel = new QLabel(bookmarkDialog);
    nameLabel->setText(tr("Name"));
    addressLabel = new QLabel(bookmarkDialog);
    addressLabel->setText(tr("Web address"));

    // Input boxes
    nameBox = new QLineEdit(bookmarkDialog);
    nameBox->setText(name);
    addressBox = new QLineEdit(bookmarkDialog);
    addressBox->setText(address.isEmpty() ? "http://" : address);

    // Spacer above save button
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Save button
    saveButton = new QPushButton(bookmarkDialog);
    saveButton->setText(tr("Save"));
    saveButton->setDefault(true);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveClicked()));
    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);

    // Layouts
    QVBoxLayout *labelLayout = new QVBoxLayout();
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(addressLabel);

    QVBoxLayout *boxLayout = new QVBoxLayout();
    boxLayout->addWidget(nameBox);
    boxLayout->addWidget(addressBox);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    QHBoxLayout *dialogLayout = new QHBoxLayout();
    if (screenGeometry.width() < screenGeometry.height()) {
        dialogLayout->setDirection(QBoxLayout::TopToBottom);
    }

    QVBoxLayout *saveButtonLayout = new QVBoxLayout();
    if (screenGeometry.width() > screenGeometry.height()) {
        saveButtonLayout->addItem(verticalSpacer);
    }
    saveButtonLayout->addWidget(buttonBox);

    // Pack all layouts together
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addItem(labelLayout);
    horizontalLayout->addItem(boxLayout);

    dialogLayout->addItem(horizontalLayout);
    dialogLayout->addItem(saveButtonLayout);

    bookmarkDialog->setLayout(dialogLayout);
    bookmarkDialog->setAttribute(Qt::WA_DeleteOnClose);
    bookmarkDialog->show();
}

void InternetRadioWindow::onSaveClicked()
{
    if (nameBox->text().isEmpty()) {
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("Unable to add empty bookmark"));
#else
        QMessageBox::critical(this, tr("Error"), tr("Unable to add empty bookmark"));
#endif
    } else {
        if (addressBox->text().indexOf("://") > 0 && !addressBox->text().endsWith("://")) {
            bookmarkDialog->close();

            GHashTable* metadata = mafw_metadata_new();
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_TITLE, nameBox->text().toUtf8().data());
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_URI, addressBox->text().toUtf8().data());
            mafw_metadata_add_str(metadata, MAFW_METADATA_KEY_MIME, "audio/unknown");

            if (bookmarkObjectId.isEmpty())
                mafwRadioSource->createObject("iradiosource::", metadata);
            else
                mafwRadioSource->setMetadata(bookmarkObjectId.toUtf8(), metadata);

            mafw_metadata_release(metadata);

#ifdef Q_WS_MAEMO_5
            QMaemo5InformationBox::information(this, tr("Media bookmark saved"));
#endif

#ifdef MAFW
            this->listStations();
#endif
        } else {
#ifdef Q_WS_MAEMO_5
            QMaemo5InformationBox::information(this, tr("Invalid URL"));
#else
            QMessageBox::critical(this, tr("Error"), tr("Invalid URL"));
#endif
        }
    }
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
    connect(mafwRadioSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllStations(uint, int, uint, QString, GHashTable*, QString)), Qt::UniqueConnection);

    browseAllStationsId = mafwRadioSource->sourceBrowse("iradiosource::", false, NULL, "+title",
                                                        MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                         MAFW_METADATA_KEY_URI,
                                                                         MAFW_METADATA_KEY_MIME),
                                                        0, MAFW_SOURCE_BROWSE_ALL);
}

void InternetRadioWindow::browseAllStations(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString)
{
    if (browseId != browseAllStationsId) return;

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

        if (mime.contains("video")) return;

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown station)");

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        URI = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown)");

        QListWidgetItem *item = ui->listWidget->item(index);

        item->setText(title);
        item->setData(UserRoleSongTitle, title);
        item->setData(UserRoleValueText, URI);
        item->setData(UserRoleObjectID, objectId);
        item->setData(UserRoleSongDuration, Duration::Blank);
    }

    if (remainingCount == 0) {
#ifdef Q_WS_MAEMO_5
        this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
        qDebug() << "disconnecting InternetRadioWindow from signalSourceBrowseResult";
        disconnect(mafwRadioSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
                   this, SLOT(browseAllStations(uint, int, uint, QString, GHashTable*, QString)));
    }

}

void InternetRadioWindow::onContainerChanged()
{
    this->listStations();
}
#endif

void InternetRadioWindow::orientationChanged()
{
    ui->listWidget->scroll(1, 1);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55),
                               ui->indicator->width(),ui->indicator->height());
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
