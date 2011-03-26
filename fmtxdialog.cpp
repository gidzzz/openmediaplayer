/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Timur Kristof

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

#include "fmtxdialog.h"

FMTXDialog::FMTXDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMTXDialog),
    selector(new FreqPickSelector(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    freqButton = new QMaemo5ValueButton(" " + QString(dgettext("osso-fm-transmitter", "fmtx_fi_frequency")), this);
    freqButton->setValueLayout(QMaemo5ValueButton::ValueBesideText);
    freqButton->setPickSelector(selector);
    ui->fmtxCheckbox->setText(dgettext("osso-fm-transmitter", "fmtx_fi_fmtx_on_off"));
    this->setWindowTitle(dgettext("osso-fm-transmitter", "fmtx_ti_fm_transmitter"));
#else
    freqButton = new QPushButton("Frequency", this);
#endif
    if (!QSettings().contains("FMTX/overrideChecks"))
        QSettings().setValue("FMTX/overrideChecks", false);
    ui->gridLayout->addWidget(freqButton, 1, 0, 1, 1);
    this->orientationChanged();
    fmtxState = new GConfItem("/system/fmtx/enabled");
    fmtxFrequency = new GConfItem("/system/fmtx/frequency");
    QString state = selector->getValue("state").toString();
    if (state == "enabled")
        ui->fmtxCheckbox->setChecked(true);
    else if (state == "n/a") {
        ui->fmtxCheckbox->setEnabled(false);
#ifdef Q_WS_MAEMO_5
        ui->fmtxCheckbox->setText(dgettext("osso-fm-transmitter", "fmtx_ni_disabled"));
#endif
    }
    connect(fmtxState, SIGNAL(valueChanged()), this, SLOT(onStateChanged()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onSaveClicked()));
    connect(ui->fmtxCheckbox, SIGNAL(clicked()), this, SLOT(onCheckboxClicked()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
}

FMTXDialog::~FMTXDialog()
{
    delete ui;
}

void FMTXDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
}

void FMTXDialog::onSaveClicked()
{
    int frequencyValue = selector->selectedFreq() * 1000;
    uint ufreq = selector->selectedFreq() * 1000;
    fmtxFrequency->set(frequencyValue);
#ifdef DEBUG
    qDebug() << "Selected Frequency:" << QString::number(frequencyValue);
#endif
    selector->setValue("frequency", ufreq);
    if(ui->fmtxCheckbox->isChecked())
        selector->setValue("state", "enabled");
    else
        selector->setValue("state", "disabled");
    this->close();
}

void FMTXDialog::onStateChanged()
{
    if(fmtxState->value().toBool())
        ui->fmtxCheckbox->setChecked(true);
    else
        ui->fmtxCheckbox->setChecked(false);
}

void FMTXDialog::orientationChanged()
{
    ui->gridLayout->removeWidget(ui->buttonBox);
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        this->setFixedHeight(230);
        ui->gridLayout->addWidget(ui->buttonBox, 3, 0, 1, ui->gridLayout->columnCount()); // portrait
        ui->buttonBox->setSizePolicy(QSizePolicy::MinimumExpanding, ui->buttonBox->sizePolicy().verticalPolicy());
    } else {
        ui->buttonBox->setSizePolicy(QSizePolicy::Maximum, ui->buttonBox->sizePolicy().verticalPolicy());
        ui->gridLayout->addWidget(ui->buttonBox, 1, 1, 1, 1, Qt::AlignBottom); // landscape
        this->setFixedHeight(160);
    }
}

void FMTXDialog::onCheckboxClicked()
{
#ifdef Q_WS_MAEMO_5
    if (!QSettings().value("FMTX/overrideChecks").toBool()) {
        QString startable = selector->getValue("startable").toString();
        if (startable != "true")
            ui->fmtxCheckbox->setChecked(false);

        if (startable == "true")
            return;
        else if (startable == "Headphones are connected") {
            this->showErrorNote(dgettext("osso-fm-transmitter", "fmtx_ni_cable_error"));
        }
        else if (startable == "Usb device is connected") {
            this->showErrorNote(dgettext("osso-fm-transmitter", "fmtx_ni_usb_error"));
        }
    }
#endif
}

void FMTXDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Backspace)
        this->close();
}

void FMTXDialog::showErrorNote(QString error)
{
    QMaemo5InformationBox *box = new QMaemo5InformationBox(this);
    box->setAttribute(Qt::WA_DeleteOnClose);

    QWidget *widget = new QWidget(box);
    QSpacerItem *spacer = new QSpacerItem(90, 20, QSizePolicy::Fixed, QSizePolicy::Maximum);

    QLabel *errorLabel = new QLabel(box);

    QHBoxLayout *layout = new QHBoxLayout(widget);

    layout->addItem(spacer);
    layout->addWidget(errorLabel);
    layout->setSpacing(0);

    widget->setLayout(layout);

    // Bad padding in default widget, use tabbing to cover it up, sigh :/
    errorLabel->setText("\n" + error + "\n\n");
    errorLabel->setAlignment(Qt::AlignLeft);
    errorLabel->setWordWrap(true);

    box->setWidget(widget);
    box->setTimeout(QMaemo5InformationBox::NoTimeout);

    connect(box, SIGNAL(clicked()), this, SLOT(close()));

    box->exec();
}
