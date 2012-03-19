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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));

    this->setAttribute(Qt::WA_DeleteOnClose);
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));

    if (QSettings().value("main/headsetButtonAction").toString() == "next")
        ui->headsetButtonAction->setCurrentIndex(0);
    else if (QSettings().value("main/headsetButtonAction").toString() == "previous")
        ui->headsetButtonAction->setCurrentIndex(1);
    else if (QSettings().value("main/headsetButtonAction").toString() == "play")
        ui->headsetButtonAction->setCurrentIndex(2);
    else if (QSettings().value("main/headsetButtonAction").toString() == "stop")
        ui->headsetButtonAction->setCurrentIndex(3);
    else if (QSettings().value("main/headsetButtonAction").toString() == "none")
        ui->headsetButtonAction->setCurrentIndex(4);

    ui->stopCheckBox->setChecked(QSettings().value("main/stopOnExit", true).toBool());
    ui->lyricsCheckBox->setChecked(QSettings().value("lyrics/enable", false).toBool());
    ui->foldersCheckBox->setChecked(QSettings().value("main/openFolders", false).toBool());
    ui->appendCheckBox->setChecked(QSettings().value("main/appendSongs", false).toBool());
    ui->slidersCheckBox->setChecked(QSettings().value("main/lazySliders", false).toBool());
    ui->fmtxCheckBox->setChecked(QSettings().value("FMTX/overrideChecks", false).toBool());

    ui->playlistSizeBox->setText(QSettings().value("music/playlistSize", 30).toString());
    ui->playlistSizeBox->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]{0,3}"), this));

    this->orientationChanged();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    if (ui->headsetButtonAction->currentIndex() == 0)
        QSettings().setValue("main/headsetButtonAction", "next");
    else if (ui->headsetButtonAction->currentIndex() == 1)
        QSettings().setValue("main/headsetButtonAction", "previous");
    else if (ui->headsetButtonAction->currentIndex() == 2)
        QSettings().setValue("main/headsetButtonAction", "play");
    else if (ui->headsetButtonAction->currentIndex() == 3)
        QSettings().setValue("main/headsetButtonAction", "stop");
    else if (ui->headsetButtonAction->currentIndex() == 4)
        QSettings().setValue("main/headsetButtonAction", "none");

    QSettings().setValue("main/stopOnExit", ui->stopCheckBox->isChecked());
    QSettings().setValue("lyrics/enable", ui->lyricsCheckBox->isChecked());
    QSettings().setValue("main/openFolders", ui->foldersCheckBox->isChecked());
    QSettings().setValue("main/appendSongs", ui->appendCheckBox->isChecked());
    QSettings().setValue("main/lazySliders", ui->slidersCheckBox->isChecked());
    QSettings().setValue("FMTX/overrideChecks", ui->fmtxCheckBox->isChecked());

    int playlistSize = ui->playlistSizeBox->text().toInt();
    QSettings().setValue("music/playlistSize", playlistSize ? playlistSize : 30);

    NowPlayingWindow::destroy();

    this->close();
}

void SettingsDialog::orientationChanged()
{
    ui->gridLayout->removeWidget(ui->buttonBox);
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        this->setFixedHeight(600);
        ui->gridLayout->addWidget(ui->buttonBox, 3, 0, 1, ui->gridLayout->columnCount()); // portrait
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else {
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout->addWidget(ui->buttonBox, 2, 1, 1, 1, Qt::AlignBottom); // landscape
        this->setFixedHeight(360);
    }
}
