#include "mainwindow.h"

/**************************************************************************
    Qt Media Player
    Copyright (C) 2010 Mohammad Abu-Garbeyyeh

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setButtonIcons();
    this->setLabelText();
    this->connectSignals();
    ui->listWidget->hide();
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);
    MafwRendererAdapter* mafwrenderer = new MafwRendererAdapter();
    myMusicWindow = new MusicWindow(this, mafwrenderer);
#else
    // Menu bar breaks layouts on desktop, hide it.
    ui->menuBar->hide();
    myMusicWindow = new MusicWindow(this);
#endif
#ifdef DEBUG
    ui->songsButton->setFlat(false);
    ui->videosButton->setFlat(false);
    ui->radioButton->setFlat(false);
    ui->shuffleAllButton->setFlat(false);
#endif
    myVideosWindow = new VideosWindow(this);
    myInternetRadioWindow = new InternetRadioWindow(this);
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

void MainWindow::showMusicWindow()
{
    myMusicWindow->show();
}

void MainWindow::showVideosWindow()
{
    myVideosWindow->show();
}

void MainWindow::showInternetRadioWindow()
{
    myInternetRadioWindow->show();
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
    connect(ui->songsButton, SIGNAL(clicked()), this, SLOT(showMusicWindow()));
    connect(ui->videosButton, SIGNAL(clicked()), this, SLOT(showVideosWindow()));
    connect(ui->radioButton, SIGNAL(clicked()), this, SLOT(showInternetRadioWindow()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
}

void MainWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()){
        // Landscape mode
        qDebug() << "MainWindow: Orientation changed: Landscape.";
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
        qDebug() << "MainWindow: Orientation changed: Portrait.";
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

        //QMainWindow::setCentralWidget(ui->listWidget);
    }
}

void MainWindow::showAbout()
{
    QMessageBox::information(this, tr("About"),
                             "Qt Mediaplayer for Maemo 5.\n\nCopyright 2010-2011:\nMohammad Abu-Garbeyyeh\n\
Sebastian Lauwers\nTimur Kristof\nNicolai Hess\n\nLicensed under GPLv3");
}

void MainWindow::processListClicks(QListWidgetItem* item)
{
    QString itemName = item->statusTip();
    if(itemName == "songs_button")
        this->showMusicWindow();
    else if(itemName == "videos_button")
        this->showVideosWindow();
    else if(itemName == "radio_button")
        this->showInternetRadioWindow();
    ui->listWidget->clearSelection();
}
