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
    ui->gridLayout_2->setContentsMargins(0,0,0,0);
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));

    this->setAttribute(Qt::WA_DeleteOnClose);

    QMaemo5ListPickSelector *selector;
    QStandardItemModel *model;

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("Next song")));
    model->appendRow(new QStandardItem(tr("Previous song")));
    model->appendRow(new QStandardItem(tr("Play / Pause")));
    model->appendRow(new QStandardItem(tr("Stop playback")));
    model->appendRow(new QStandardItem(tr("Do nothing")));
    selector->setModel(model);
    selector->setCurrentIndex(QSettings().value("main/headsetButtonAction").toString() == "next" ? 0 :
                                     QSettings().value("main/headsetButtonAction").toString() == "previous" ? 1 :
                                     QSettings().value("main/headsetButtonAction").toString() == "playpause" ? 2 :
                                     QSettings().value("main/headsetButtonAction").toString() == "stop" ? 3 :
                                     QSettings().value("main/headsetButtonAction").toString() == "none" ? 4 : 0);
    ui->headsetButtonBox->setPickSelector(selector);

    selector = new QMaemo5ListPickSelector;
    model = new QStandardItemModel(0, 1, selector);
    model->appendRow(new QStandardItem(tr("Automatic")));
    model->appendRow(new QStandardItem(tr("Landscape")));
    model->appendRow(new QStandardItem(tr("Portrait")));
    selector->setModel(model);
    selector->setCurrentIndex(QSettings().value("main/orientation").toString() == "automatic" ? 0 :
                                         QSettings().value("main/orientation").toString() == "landscape" ? 1 :
                                         QSettings().value("main/orientation").toString() == "portrait" ? 2 : 0);
    ui->orientationBox->setPickSelector(selector);

    ui->stopCheckBox->setChecked(QSettings().value("main/stopOnExit", true).toBool());
    ui->headsetCheckBox->setChecked(QSettings().value("main/pauseHeadset", true).toBool());
    ui->lyricsCheckBox->setChecked(QSettings().value("lyrics/enable", false).toBool());
    ui->filterCheckBox->setChecked(QSettings().value("main/playlistFilter", false).toBool());
    ui->foldersCheckBox->setChecked(QSettings().value("main/openFolders", false).toBool());
    ui->appendCheckBox->setChecked(QSettings().value("main/appendSongs", false).toBool());
    ui->slidersCheckBox->setChecked(QSettings().value("main/lazySliders", false).toBool());
    ui->fmtxCheckBox->setChecked(QSettings().value("FMTX/overrideChecks", false).toBool());

    ui->playlistSizeBox->setText(QSettings().value("music/playlistSize", 30).toString());
    ui->playlistSizeBox->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]{0,3}"), this));

    Rotator *rotator = Rotator::acquire();
    connect(rotator, SIGNAL(rotated(int,int)), this, SLOT(orientationChanged(int,int)));
    orientationChanged(rotator->width(), rotator->height());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    switch (static_cast<QMaemo5ListPickSelector*>(ui->headsetButtonBox->pickSelector())->currentIndex()) {
        case 0: QSettings().setValue("main/headsetButtonAction", "next"); break;
        case 1: QSettings().setValue("main/headsetButtonAction", "previous"); break;
        case 2: QSettings().setValue("main/headsetButtonAction", "playpause"); break;
        case 3: QSettings().setValue("main/headsetButtonAction", "stop"); break;
        case 4: QSettings().setValue("main/headsetButtonAction", "none"); break;
    }

    switch (static_cast<QMaemo5ListPickSelector*>(ui->orientationBox->pickSelector())->currentIndex()) {
        case 0: QSettings().setValue("main/orientation", "automatic"); break;
        case 1: QSettings().setValue("main/orientation", "landscape"); break;
        case 2: QSettings().setValue("main/orientation", "portrait"); break;
    }

    QSettings().setValue("main/stopOnExit", ui->stopCheckBox->isChecked());
    QSettings().setValue("main/pauseHeadset", ui->headsetCheckBox->isChecked());
    QSettings().setValue("lyrics/enable", ui->lyricsCheckBox->isChecked());
    QSettings().setValue("main/playlistFilter", ui->filterCheckBox->isChecked());
    QSettings().setValue("main/openFolders", ui->foldersCheckBox->isChecked());
    QSettings().setValue("main/appendSongs", ui->appendCheckBox->isChecked());
    QSettings().setValue("main/lazySliders", ui->slidersCheckBox->isChecked());
    QSettings().setValue("FMTX/overrideChecks", ui->fmtxCheckBox->isChecked());

    int playlistSize = ui->playlistSizeBox->text().toInt();
    QSettings().setValue("music/playlistSize", playlistSize ? playlistSize : 30);

    this->close();
}

void SettingsDialog::orientationChanged(int w, int h)
{
    ui->gridLayout->removeWidget(ui->buttonBox);
    if (w < h) { // Portrait
        this->setFixedHeight(600);
        ui->gridLayout->addWidget(ui->buttonBox, 3, 0, 1, ui->gridLayout->columnCount());
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else { // Landscape
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout->addWidget(ui->buttonBox, 2, 1, 1, 1, Qt::AlignBottom);
        this->setFixedHeight(360);
    }
}
