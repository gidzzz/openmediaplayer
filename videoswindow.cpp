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

VideosWindow::VideosWindow(QWidget *parent, MafwRendererAdapter* mra, MafwSourceAdapter* msa, MafwPlaylistAdapter* pls) :
    QMainWindow(parent),
    ui(new Ui::VideosWindow)
#ifdef MAFW
    ,mafwrenderer(mra),
    mafwTrackerSource(msa),
    playlist(pls)
#endif
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
#ifdef MAFW
    ui->indicator->setSources(this->mafwrenderer, this->mafwTrackerSource, this->playlist);
#endif
    sortByActionGroup = new QActionGroup(this);
    sortByActionGroup->setExclusive(true);
    sortByDate = new QAction(tr("Date"), sortByActionGroup);
    sortByDate->setCheckable(true);
    this->selectView();
    if(sortByDate->isChecked())
        QMainWindow::setWindowTitle(tr("Videos - latest"));
    else
        QMainWindow::setWindowTitle(tr("Videos - categories"));
    sortByCategory = new QAction(tr("Category"), sortByActionGroup);
    sortByCategory->setCheckable(true);
    this->menuBar()->addActions(sortByActionGroup->actions());
    this->connectSignals();
    this->orientationChanged();
#ifdef MAFW
    if(mafwTrackerSource->isReady())
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
    connect(ui->listWidget, SIGNAL(activated(QModelIndex)), this, SLOT(onVideoSelected()));
    connect(ui->menubar, SIGNAL(triggered(QAction*)), this, SLOT(onSortingChanged(QAction*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
}

void VideosWindow::onVideoSelected()
{
    // Placeholder function
    ui->listWidget->clearSelection();
    VideoNowPlayingWindow *window = new VideoNowPlayingWindow(this);
    window->showFullScreen();
}

void VideosWindow::onSortingChanged(QAction *action)
{
    if(action == sortByDate) {
        QMainWindow::setWindowTitle(tr("Videos - latest"));
        QSettings().setValue("Videos/Sortby", "date");
    } else if(action == sortByCategory) {
        QMainWindow::setWindowTitle(tr("Videos - categories"));
        QSettings().setValue("Videos/Sortby", "caregory");
    }
}

void VideosWindow::selectView()
{
    if(QSettings().value("Videos/Sortby").toString() == "category")
        sortByCategory->setChecked(true);
    else
        sortByDate->setChecked(true);
}

#ifdef MAFW
void VideosWindow::listVideos()
{
#ifdef DEBUG
    qDebug("Source ready");
#endif
    connect(mafwTrackerSource, SIGNAL(signalSourceBrowseResult(uint, int, uint, QString, GHashTable*, QString)),
            this, SLOT(browseAllVideos(uint, int, uint, QString, GHashTable*, QString)));

    this->browseAllVideosId = mafwTrackerSource->sourceBrowse("localtagfs::videos", false, NULL, NULL,
                                                               MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                                                MAFW_METADATA_KEY_DURATION,
                                                                                MAFW_METADATA_KEY_THUMBNAIL_URI,
                                                                                MAFW_METADATA_KEY_URI,
                                                                                ),
                                                               0, MAFW_SOURCE_BROWSE_ALL);
}

void VideosWindow::browseAllVideos(uint browseId, int, uint, QString objectId, GHashTable* metadata, QString)
{
    Q_UNUSED(objectId);
    if(browseId != browseAllVideosId)
      return;


    QString title;
    int duration = -1;
    if(metadata != NULL) {
        GValue *v;
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_TITLE);
        title = v ? QString::fromUtf8(g_value_get_string (v)) : tr("(unknown clip)");
        v = mafw_metadata_first(metadata,
                                MAFW_METADATA_KEY_DURATION);
        duration = v ? g_value_get_int (v) : -1;

        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(UserRoleSongURI, QString::fromUtf8(filename));
            }
        }
        v = mafw_metadata_first(metadata, MAFW_METADATA_KEY_THUMBNAIL_URI);
        if(v != NULL) {
            const gchar* file_uri = g_value_get_string(v);
            gchar* filename = NULL;
            if(file_uri != NULL && (filename = g_filename_from_uri(file_uri, NULL, NULL)) != NULL) {
                item->setData(Qt::DecorationRole, QString::fromUtf8(filename));
            }
        }

        item->setText(title + "\n" + QString::number(duration));
        if(item->data(Qt::DecorationRole).isNull())
            item->setIcon(QIcon(defaultVideoImage));
        else
            item->setIcon(QIcon(item->data(Qt::DecorationRole).toString()));
        ui->listWidget->addItem(item);
  }
}
#endif

void VideosWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    ui->indicator->raise();
}

void VideosWindow::focusInEvent(QFocusEvent *)
{
    ui->indicator->triggerAnimation();
}

void VideosWindow::focusOutEvent(QFocusEvent *)
{
    ui->indicator->stopAnimation();
}
