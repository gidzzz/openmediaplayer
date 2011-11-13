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
    if (QSettings().value("main/onApplicationExit").toString() == "do-nothing")
        ui->onExitBox->setCurrentIndex(1);
    if (QSettings().contains("lyrics/enable"))
        if (QSettings().value("lyrics/enable").toBool())
            ui->lyricsCheckBox->setChecked(true);
    if (QSettings().contains("main/openFolders"))
        if (QSettings().value("main/openFolders").toBool())
            ui->foldersCheckBox->setChecked(true);
    if (QSettings().contains("main/lazySliders"))
        if (QSettings().value("main/lazySliders").toBool())
            ui->slidersCheckBox->setChecked(true);
    if (QSettings().contains("FMTX/overrideChecks"))
        if (QSettings().value("FMTX/overrideChecks").toBool())
            ui->fmtxCheckBox->setChecked(true);

    ui->playlistSizeBox->setText(QSettings().value("music/playlistSize").toString());
    ui->playlistSizeBox->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]{0,3}"), this));

    this->orientationChanged();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    if (ui->onExitBox->currentIndex() == 0)
        QSettings().setValue("main/onApplicationExit", "stop-playback");
    else if (ui->onExitBox->currentIndex() == 1)
        QSettings().setValue("main/onApplicationExit", "do-nothing");

    QSettings().setValue("lyrics/enable", ui->lyricsCheckBox->isChecked());
    QSettings().setValue("main/openFolders", ui->foldersCheckBox->isChecked());
    QSettings().setValue("main/lazySliders", ui->slidersCheckBox->isChecked());
    NowPlayingWindow::destroy();

    int playlistSize = ui->playlistSizeBox->text().toInt();
    if (playlistSize == 0) playlistSize = 30;
    QSettings().setValue("music/playlistSize", playlistSize);

    QSettings().setValue("FMTX/overrideChecks", ui->fmtxCheckBox->isChecked());

    this->close();
}

void SettingsDialog::orientationChanged()
{
    ui->gridLayout->removeWidget(ui->buttonBox);
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        this->setFixedHeight(320);
        ui->gridLayout->addWidget(ui->buttonBox, 3, 0, 1, ui->gridLayout->columnCount()); // portrait
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else {
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout->addWidget(ui->buttonBox, 2, 1, 1, 1, Qt::AlignBottom); // landscape
        this->setFixedHeight(240);
    }
}
