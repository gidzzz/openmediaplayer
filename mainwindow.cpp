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

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setButtonIcons();
    this->setLabelText();
    ui->listWidget->hide();

#ifdef MAFW
    TAGSOURCE_AUDIO_PATH = "localtagfs::music/songs";
    TAGSOURCE_VIDEO_PATH = "localtagfs::videos";
    RADIOSOURCE_PATH = "iradiosource::";
    mafwrenderer = new MafwRendererAdapter();
    mafwTrackerSource = new MafwSourceAdapter("Mafw-Tracker-Source");
    mafwRadioSource = new MafwSourceAdapter("Mafw-IRadio-Source");
    playlist = new MafwPlaylistAdapter(this, this->mafwrenderer);
#endif

#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);

    ui->songCountL->clear();
    ui->videoCountL->clear();
    ui->startionCountL->clear();
#else
    // Menu bar breaks layouts on desktop, hide it.
    ui->menuBar->hide();
#endif

#ifdef MAFW
    myMusicWindow = new MusicWindow(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
    myVideosWindow = new VideosWindow(this, this->mafwrenderer, this->mafwTrackerSource, this->playlist);
    myInternetRadioWindow = new InternetRadioWindow(this, this->mafwrenderer, this->mafwRadioSource, this->playlist);
#else
    myMusicWindow = new MusicWindow(this);
    myVideosWindow = new VideosWindow(this);
    myInternetRadioWindow = new InternetRadioWindow(this);
#endif

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55), 112, 70);
    this->connectSignals();

#ifdef DEBUG
    ui->songsButton->setFlat(false);
    ui->videosButton->setFlat(false);
    ui->radioButton->setFlat(false);
    ui->shuffleAllButton->setFlat(false);
#endif

    ui->indicator->setSources(this->mafwrenderer, this->mafwTrackerSource, this->playlist);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage(backgroundImage));
}

void MainWindow::setButtonIcons()
{
    ui->songsButton->setIcon(QIcon(musicIcon));
    ui->videosButton->setIcon(QIcon(videosIcon));
    ui->radioButton->setIcon(QIcon(radioIcon));
    ui->shuffleAllButton->setIcon(QIcon(shuffleIcon));
}

void MainWindow::setLabelText()
{
    ui->songsButtonLabel->setText(tr("Music"));
    ui->videosButtonLabel->setText(tr("Videos"));
    ui->radioButtonLabel->setText(tr("Internet Radio"));
    ui->shuffleLabel->setText(tr("Shuffle all songs"));
}

void MainWindow::connectSignals()
{
    connect(ui->songsButton, SIGNAL(clicked()), myMusicWindow, SLOT(show()));
    connect(ui->videosButton, SIGNAL(clicked()), myVideosWindow, SLOT(show()));
    connect(ui->radioButton, SIGNAL(clicked()), myInternetRadioWindow, SLOT(show()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));
    // TODO: Connect this to a slot.
    // connect(ui->indicator, SIGNAL(clicked()), this, SLOT();
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#ifdef MAFW
    connect(mafwTrackerSource, SIGNAL(sourceReady()), this, SLOT(trackerSourceReady()));
    connect(mafwRadioSource, SIGNAL(sourceReady()), this, SLOT(radioSourceReady()));
    connect(mafwTrackerSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countAudioVideoResult(QString, GHashTable*, QString)));
    connect(mafwRadioSource, SIGNAL(signalMetadataResult(QString, GHashTable*, QString)),
            this, SLOT(countRadioResult(QString, GHashTable*, QString)));
#endif
}

void MainWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()){
        // Landscape mode
#ifdef DEBUG
        qDebug() << "MainWindow: Orientation changed: Landscape.";
#endif
        if(!ui->listWidget->isHidden()) {
            ui->listWidget->hide();
        ui->songsButton->show();
        ui->songsButtonLabel->show();
        ui->videosButton->show();
        ui->videosButtonLabel->show();
        ui->radioButton->show();
        ui->radioButtonLabel->show();
        ui->shuffleAllButton->show();
        ui->shuffleLabel->show();
        ui->songCountL->show();
        ui->videoCountL->show();
        ui->startionCountL->show();
        }
    } else {
        // Portrait mode
#ifdef DEBUG
        qDebug() << "MainWindow: Orientation changed: Portrait.";
#endif
        ui->songsButton->hide();
        ui->songsButtonLabel->hide();
        ui->videosButton->hide();
        ui->videosButtonLabel->hide();
        ui->radioButton->hide();
        ui->radioButtonLabel->hide();
        ui->shuffleAllButton->hide();
        ui->shuffleLabel->hide();
        ui->songCountL->hide();
        ui->videoCountL->hide();
        ui->startionCountL->hide();
        ui->listWidget->setGeometry(QRect(0, 0, 480, 800));
        if(ui->listWidget->isHidden())
            ui->listWidget->show();
    }
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55),
                               ui->indicator->width(),ui->indicator->height());
    ui->indicator->raise();
}

void MainWindow::showAbout()
{
    QMessageBox::information(this, tr("About"),
                             "Qt Mediaplayer for Maemo 5.\n\nCopyright 2010-2011:\nMohammad Abu-Garbeyyeh\n\
Sebastian Lauwers\nTimur Kristof\nNicolai Hess\n\nLicensed under GPLv3\n\nBuild Date: " + QString(__DATE__ ) + " "  + QString(__TIME__ ));
}

void MainWindow::processListClicks(QListWidgetItem* item)
{
    QString itemName = item->statusTip();
    if(itemName == "songs_button")
        myMusicWindow->show();
    else if(itemName == "videos_button")
        myVideosWindow->show();
    else if(itemName == "radio_button")
        myInternetRadioWindow->show();
    ui->listWidget->clearSelection();
}

#ifdef MAFW
void MainWindow::trackerSourceReady()
{
    this->countSongs();
    this->countVideos();
}

void MainWindow::radioSourceReady()
{
    countRadioStations();
}

void MainWindow::countSongs()
{
    mafwTrackerSource->getMetadata(TAGSOURCE_AUDIO_PATH,
                                   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1));
}

void MainWindow::countVideos()
{
    mafwTrackerSource->getMetadata(TAGSOURCE_VIDEO_PATH,
                                   MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1));
}

void MainWindow::countRadioStations()
{
    mafwRadioSource->getMetadata(RADIOSOURCE_PATH,
                                 MAFW_SOURCE_LIST(MAFW_METADATA_KEY_CHILDCOUNT_1));
}


void MainWindow::countAudioVideoResult(QString objectId, GHashTable* metadata, QString error)
{
    int count = -1;
    GValue *v;
    v = mafw_metadata_first(metadata,
                            MAFW_METADATA_KEY_CHILDCOUNT_1);
    if(v)
        count = g_value_get_int(v);

    QString countStr;
    countStr.setNum(count);

    if(objectId == TAGSOURCE_AUDIO_PATH) {
        if(count == 1)
            countStr.append(" " + tr("song"));
        else if(count == -1)
            countStr = tr("(no songs)");
        else
            countStr.append(" " + tr("songs"));
        ui->songCountL->setText(countStr);
    } else if(objectId == TAGSOURCE_VIDEO_PATH) {
        if(count == 1)
            countStr.append(" " + tr("clip"));
        else if(count == -1)
            countStr = tr("(no videos)");
        else
            countStr.append(" " + tr("clips"));
        ui->videoCountL->setText(countStr);
    }
    if(!error.isEmpty())
        qDebug() << error;
}

void MainWindow::countRadioResult(QString, GHashTable* metadata, QString error)
{
    int count = -1;
    GValue *v;
    v = mafw_metadata_first(metadata,
                            MAFW_METADATA_KEY_CHILDCOUNT_1);
    if(v)
        count = g_value_get_int(v);

    if(count == 1)
        ui->startionCountL->setText(QString::number(count) + " " + tr("station"));
    else if(count == -1)
        ui->startionCountL->setText(tr("(no stations)"));
    else
        ui->startionCountL->setText(QString::number(count) + " " + tr("stations"));
    if(!error.isEmpty())
        qDebug() << error;
}
#endif

void MainWindow::focusInEvent(QFocusEvent *)
{
    qDebug() << "MainWindow: focused.";
    ui->indicator->triggerAnimation();
}

void MainWindow::focusOutEvent(QFocusEvent *)
{
    qDebug() << "MainWindow: focus lost";
    ui->indicator->stopAnimation();
}
