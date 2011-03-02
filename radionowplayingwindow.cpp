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

#include "radionowplayingwindow.h"
#include "ui_radionowplayingwindow.h"

RadioNowPlayingWindow::RadioNowPlayingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RadioNowPlayingWindow)
{
    ui->setupUi(this);
    ui->volumeSlider->hide();
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    this->setIcons();
    volumeTimer = new QTimer(this);
    volumeTimer->setInterval(3000);
    this->connectSignals();
    this->orientationChanged();
}

RadioNowPlayingWindow::~RadioNowPlayingWindow()
{
    delete ui;
}

void RadioNowPlayingWindow::connectSignals()
{
    connect(ui->volumeButton, SIGNAL(clicked()), this, SLOT(toggleVolumeSlider()));
    connect(ui->volumeButton, SIGNAL(clicked()), volumeTimer, SLOT(start()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#ifdef Q_WS_MAEMO_5
    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
#endif
    connect(ui->nextButton, SIGNAL(pressed()), this, SLOT(onNextButtonPressed()));
    connect(ui->nextButton, SIGNAL(released()), this, SLOT(onNextButtonPressed()));
    connect(ui->prevButton, SIGNAL(pressed()), this, SLOT(onPrevButtonPressed()));
    connect(ui->prevButton, SIGNAL(released()), this, SLOT(onPrevButtonPressed()));
}

void RadioNowPlayingWindow::setIcons()
{
    ui->prevButton->setIcon(QIcon(prevButtonIcon));
    ui->playButton->setIcon(QIcon(playButtonIcon));
    ui->nextButton->setIcon(QIcon(nextButtonIcon));
    ui->volumeButton->setIcon(QIcon(volumeButtonIcon));
}

void RadioNowPlayingWindow::toggleVolumeSlider()
{
    if(ui->volumeSlider->isHidden()) {
        ui->buttonsWidget->hide();
        ui->volumeSlider->show();
    } else {
        ui->volumeSlider->hide();
        ui->buttonsWidget->show();
        if(volumeTimer->isActive())
            volumeTimer->stop();
    }
}

void RadioNowPlayingWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        ui->mainLayout->setDirection(QBoxLayout::LeftToRight);
        ui->buttonsLayout->addItem(ui->horizontalSpacer);
        ui->volumeWidget->show();
        ui->spacerWidget->show();
    } else {
        ui->volumeWidget->hide();
        ui->buttonsLayout->removeItem(ui->horizontalSpacer);
        ui->mainLayout->setDirection(QBoxLayout::TopToBottom);
        ui->spacerWidget->hide();
    }
}

#ifdef Q_WS_MAEMO_5
void RadioNowPlayingWindow::showFMTXDialog()
{
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
}
#endif

void RadioNowPlayingWindow::onNextButtonPressed()
{
    if(ui->nextButton->isDown())
        ui->nextButton->setIcon(QIcon(nextButtonPressedIcon));
    else
        ui->nextButton->setIcon(QIcon(nextButtonIcon));
}

void RadioNowPlayingWindow::onPrevButtonPressed()
{
    if(ui->prevButton->isDown())
        ui->prevButton->setIcon(QIcon(prevButtonPressedIcon));
    else
        ui->prevButton->setIcon(QIcon(prevButtonIcon));
}
