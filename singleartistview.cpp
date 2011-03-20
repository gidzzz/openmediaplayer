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

#include "singleartistview.h"
#include "ui_singleartistview.h"

SingleArtistView::SingleArtistView(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa, MafwPlaylistAdapter* pls) :
    QMainWindow(parent),
    ui(new Ui::SingleArtistView)
#ifdef MAFW
    ,mafwrenderer(mra),
    mafwTrackerSource(msa),
    playlist(pls)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->searchHideButton->setIcon(QIcon::fromTheme("general_close"));
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
#ifdef MAFW
    ui->indicator->setSources(this->mafwrenderer, this->mafwTrackerSource, this->playlist);
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllAlbums(uint, int, uint, QString, GHashTable*, QString)));
    connect(ui->albumList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(onAlbumSelected(QListWidgetItem*)));
#endif
    ui->searchWidget->hide();

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    connect(ui->searchEdit, SIGNAL(textChanged(QString)), this, SLOT(onSearchTextChanged(QString)));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchWidget, SLOT(hide()));
    connect(ui->searchHideButton, SIGNAL(clicked()), ui->searchEdit, SLOT(clear()));
    this->orientationChanged();
}

SingleArtistView::~SingleArtistView()
{
    delete ui;
}

void SingleArtistView::browseAlbum(QString album)
{
#ifdef MAFW
    this->artistObjectId = album;
    this->listAlbums();
#else
    Q_UNUSED(album)
#endif
}

#ifdef MAFW
void SingleArtistView::listAlbums()
{
    ui->albumList->clear();
    QListWidgetItem *shuffleButton = new QListWidgetItem(ui->albumList);
    shuffleButton->setIcon(QIcon(shuffleIcon124));
    shuffleButton->setText(tr("Shuffle songs"));
    shuffleButton->setData(Qt::UserRole, "shuffle");

    this->browseAllAlbumsId = mafwTrackerSource->sourceBrowse(this->artistObjectId.toUtf8(), false, NULL, NULL,
                                                              MAFW_SOURCE_LIST(MAFW_METADATA_KEY_ALBUM,
                                                                               MAFW_METADATA_KEY_ARTIST,
                                                                               MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI),
                                                              0, MAFW_SOURCE_BROWSE_ALL);
}

void SingleArtistView::browseAllAlbums(uint browseId, int remainingCount, uint, QString objectId, GHashTable* metadata, QString error)
{
    if(browseId != browseAllAlbumsId)
        return;

    QString albumTitle;
    QString artist;
    QString albumArt;
    QListWidgetItem *item = new QListWidgetItem();
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ALBUM);
        albumTitle = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown album)";

        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_ARTIST);
        artist = v ? QString::fromUtf8(g_value_get_string(v)) : "(unknown artist)";

        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_ALBUM_ART_MEDIUM_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleAlbumArt, filename);
            }
        }
    }

    item->setData(UserRoleSongAlbum, albumTitle);
    item->setData(UserRoleAlbumCount, artist);
    item->setData(UserRoleObjectID, objectId);
    item->setText(albumTitle);
    if(item->data(UserRoleAlbumArt).isNull())
        item->setIcon(QIcon(defaultAlbumArtMedium));
    else {
        QPixmap icon(item->data(UserRoleAlbumArt).toString());
        item->setIcon(QIcon(icon.scaled(124, 124)));
    }
    ui->albumList->addItem(item);
    if(!error.isEmpty())
        qDebug() << error;
#ifdef Q_WS_MAEMO_5
        if(remainingCount != 0)
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, true);
        else
            this->setAttribute(Qt::WA_Maemo5ShowProgressIndicator, false);
#endif
}
#endif

void SingleArtistView::onAlbumSelected(QListWidgetItem *item)
{
#ifdef MAFW
    if (item->data(Qt::UserRole).toString() == "shuffle")
        return;
    else {
        SingleAlbumView *albumView = new SingleAlbumView(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
        albumView->setAttribute(Qt::WA_DeleteOnClose);
        albumView->browseAlbumByObjectId(item->data(UserRoleObjectID).toString());
        albumView->setWindowTitle(item->text());
        albumView->show();
    }
    ui->albumList->clearSelection();
#endif
}


void SingleArtistView::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void SingleArtistView::onSearchTextChanged(QString text)
{
    if (!ui->indicator->isHidden())
        ui->indicator->hide();

    for (int i=0; i < ui->albumList->count(); i++) {
        if (ui->albumList->item(i)->text().toLower().indexOf(text.toLower()) == -1)
            ui->albumList->item(i)->setHidden(true);
        else
            ui->albumList->item(i)->setHidden(false);
    }

    if (text.isEmpty()) {
        ui->searchWidget->hide();
        if (ui->indicator->isHidden())
            ui->indicator->show();
    }
}

void SingleArtistView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Left || e->key() == Qt::Key_Right || e->key() == Qt::Key_Backspace)
        return;
    else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down && !ui->searchWidget->isHidden())
        ui->albumList->setFocus();
    else {
        ui->albumList->clearSelection();
        if (ui->searchWidget->isHidden())
            ui->searchWidget->show();
        if (!ui->searchEdit->hasFocus())
            ui->searchEdit->setText(ui->searchEdit->text() + e->text());
        ui->searchEdit->setFocus();
    }
}
